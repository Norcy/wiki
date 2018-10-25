# 一、前言
对比爱奇艺，腾讯视频 App 启动速度较慢，于是我们在腾讯视频 iPhone 端的5.5.0版本进行了启动优化。对比前一个版本，在 iPhone5 上，页面有缓存的情况下从10.637s提升到5.644s，无缓存的情况下从8.717s提升到5.631秒

本文对此次优化工作进行总结沉淀，将大家遇到的坑和学到的东西记录下来，时间仓促，可能不是很完整，总之先做备忘，后续再完善

大概分为三个部分：

1. [iOS 启动优化实践（一）启动任务的架构](http://km.oa.com/group/27088/articles/show/299853)
2. [iOS 启动优化实践（二）工具与技巧](http://km.oa.com/group/27088/articles/show/299854)
3. [iOS 启动优化实践（三）多线程安全](http://km.oa.com/group/27088/articles/show/299856)

第一部分主要对启动任务的架构进行介绍

# 二、准备工作
对 App 启动速度进行优化之前，我们先做了一次准备工作，就是将 AppDelegate 中的代码进行整理归类

优化前的 AppDelegate.h 大概 300 行代码，接口乱七八糟；而 AppDelegate.m 大概 3000 行代码，其中掺杂了启动任务、上报、Crash 处理、网络处理等等

我们将与 AppDelegate 无关的代码（包括众多启动任务）抽离，放到对应的模块之后，AppDelegate 只保留了最基本的接口和实现，保证没有一个函数和变量是多余的，清爽了许多，也为后续整理启动任务提供了方便

![](http://7xsd8c.com1.z0.glb.clouddn.com/FastAppDelegate_h.png)

![](http://7xsd8c.com1.z0.glb.clouddn.com/FastAppDelegate_m.png)

# 三、启动任务的分类
优化前，启动任务就是直接丢到 `didFinishLaunchingWithOptions` 中去做

```objc
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // 任务1
    // 任务2
    // 任务3
    // ...(此处省略500行)
	return YES;
}
```

这种做法虽然简单，但是有以下缺点：

+ 所有任务都在**主线程**执行，且**同步**执行，必然会影响启动速度
+ 任务没有**优先级**的概念，都是在 `didFinishLaunchingWithOptions` 结束前完成，低优先级的任务势必在启动期间占用了系统资源

针对以上缺点，我们从以下维度对启动任务进行分类
## 1.1 主线程 or 子线程
主线程执行的任务势必会影响 UI 的加载，所以主线程的任务能少则少

在我们看来，主线程执行的任务分为2类：

1. 一定要在主线程处理的任务（比如操作 UIKit 对象、创建 App 的主框架、设置 Push 通知）
2. 业务上的超高优先级，必须提前优先做（比如登录、Crash 上报）

除此之外，其他任务应该全部丢到子线程处理

## 1.2 同步 or 异步
这里我们使用了 GCD 技术，这里的同步和异步对应着 GCD 里的 `dispatch_sync` 和 `dispatch_async` 方法

+ 对于在主线程执行的任务，如果后续业务对其依赖很大，则同步（比如创建 App 的主框架）；否则异步处理（比如注册 Push 通知）
+ 对于在子线程执行的任务，则全部异步处理

__至于为什么要使用 GCD？__

+ 易用: GCD 比之 thread 跟简单易用。由于 GCD 基于 work unit 而非像 thread 那样基于运算，所以 GCD 可以控制诸如等待任务结束、监视文件描述符、周期执行代码以及工作挂起等任务。基于 block 的血统导致它能极为简单得在不同代码作用域之间传递上下文
+ 效率: GCD 被实现得如此轻量和优雅，使得它在很多地方比之专门创建消耗资源的线程更实用且快速。这关系到易用性：导致 GCD 易用的原因有一部分在于你可以不用担心太多的效率问题而仅仅使用它就行了
+ 性能: GCD 自动根据系统负载来增减线程数量，这就减少了上下文切换以及增加了计算效率

使用 GCD 需要为其选择一个队列，主线程执行的任务必然选择主队列，那么子线程执行的异步任务，应该选择什么队列呢？

GCD 的异步队列有串行队列和并发队列2种

串行队列效率较低，而并发队列会有一个缺点：当某个 block 所在线程被锁住时，并发队列会创建大量线程以至于占用了过多资源而影响到主线程

基于此，我们选择了 [YYDispatchQueuePool](https://github.com/ibireme/YYDispatchQueuePool) 来解决这个问题

## 1.3 优先级 = 时间 + 资源调度
时间上看，不同优先级的任务在不同的时间点执行，主要有2个时间点：`didFinishLaunchingWithOptions` 结束前和 App 主框架加载完毕之后(TabbarController 的 viewDidAppear)

而资源调度方面，主要体现在子线程任务上，GCD 的 `dispatch_queue_t` 创建时，可以指定队列的优先级 `NSQualityOfService`，在系统资源比较紧张的时候，优先级较高的 `dispatch_queue_t` 会优先获得系统资源

__关于 NSQualityOfService__

`NSQualityOfService` 的定义如下，注意这个是 iOS8 之后才有的，不过由于是基本数据类型，只要编译的环境有 iOS8 及以上的 SDK，那么编译后在 iOS7 上仍然是可用的

```objc
typedef NS_ENUM(NSInteger, NSQualityOfService) 
{
    NSQualityOfServiceUserInteractive = 0x21,
    NSQualityOfServiceUserInitiated = 0x19,
    NSQualityOfServiceUtility = 0x11,
    NSQualityOfServiceBackground = 0x09,
    NSQualityOfServiceDefault = -1
} NS_ENUM_AVAILABLE(10_10, 8_0);
```

## 1.4 启动任务的分类
任务的优先级体现在时间上和资源调度上，结合以上2个维度以及工程业务的需要，我们将启动任务分为以下5类

|    启动任务    | 时间 | 资源调度 | 备注 |
| ------------- | --- | ------- | --- |
| 主线程+同步+P0 |  didFinishLaunching | 主线程 | 超高优先级，需要卡主线程 UI，会影响启动速度，谨慎添加 |
| 主线程+异步+P1 | TabBarController viewDidAppear 延时1.5s | 主线程 | 优先级较低，但不得不在主线程执行 | 
| 子线程+异步+P0 | TabBarController viewDidAppear 不延时| NSQualityOfServiceUserInitiated 级别的 GCD 队列 | 优先级较高，可以不卡主线程做，其他业务 or UI 对其有依赖，能尽量早做就早做（比如 Vip 业务） |
| 子线程+异步+P1 | TabBarController viewDidAppear 延时1.0s| NSQualityOfServiceDefault 级别的 GCD 队列 | 优先级中等，可以不卡主线程做，只要在特定场景（比如播放、切换帐号、进个人中心）之前完成就行 |
| 子线程+异步+P2 | TabBarController viewDidAppear 延时3.0s| NSQualityOfServiceBackground 级别的 GCD 队列 | 优先级较低，可以不卡主线程做，其他业务对其无依赖，基本随便找个时间做就行（比如上报竞品信息、上报用户安装信息）|

`[TabBarController viewDidAppear]` 执行时，App 的主框架已基本搭建完成，用户已经可以直观感受到界面的存在，所以现在影响 App 启动速度的启动任务，只剩下【主线程+同步+P0】类型的任务，这样 App 启动就变快了；至于各个优先级的任务具体延时多少秒，需要花时间微调

另外，考虑到后续的扩展性，[新增一个启动任务时，如何确定其类型？](http://naotu.baidu.com/file/a0aeed9da50274635a9f9f9ef32a5a6c?token=0ea6fc2d13fdbb32)主要可以从两个维度去分析，一个是任务本身的性质，另一个是其与其他任务的耦合性

# 四、启动任务管理框架
主要涉及4个类：

+ AppDelegate
+ TabBarController
+ AppLaunchTaskList（启动任务列表基类） 
+ AppLaunchTaskManager（管理 App 启动任务的调度）

类图如下：
![](http://7xsd8c.com1.z0.glb.clouddn.com/AppLaunch_Class.png)

一次启动过程的流程如下（以 MainSyncP0 和 MainAsynP1 为例，剩下的类型跟 MainAsynP1 同理）：
![](http://7xsd8c.com1.z0.glb.clouddn.com/AppLaunch_Sequence.png)


