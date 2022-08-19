+ [RunLoop 脑图](http://naotu.baidu.com/file/1048175c83ab832d33e9650c84ef2abe)
+ [RunLoop 流程](https://www.processon.com/diagraming/5d22043be4b0f42d067a58fd)

# RunLoop 相关概念
## Event Loop
事件循环模型，实现这种模型的关键点在于：如何管理事件/消息，如何让线程在没有处理消息时休眠以避免资源占用、在有消息到来时立刻被唤醒

```objc
function loop() 
{
    init();
    do 
    {
        var message = get_next_message();
        process_message(message);
    }
    while (message != quit);
}
```

线程执行了这个函数后，就会一直处于这个函数内部 “接受消息->等待->处理” 的循环中，直到这个循环结束

RunLoop 就是 OSX/iOS 平台对事件循环模型的实现，在循环中用来处理程序运行过程中出现的各种事件（比如说触摸事件、UI 刷新事件、定时器事件）和消息，从而保持程序的持续运行；而且在没有事件处理的时候，会进入睡眠模式，从而节省 CPU 资源，提高程序性能

## NSRunLoop 和 CFRunLoop
OSX/iOS 系统中，提供了两个这样的对象：NSRunLoop 和 CFRunLoop

CFRunLoop 是 Core Foundation 框架内的，它提供了纯 C 函数的 API，所有这些 API 都是线程安全的

NSRunLoop 是 Foundation 框架内的，提供了面向对象的 API，但是这些 API 不是线程安全的

其中 NSRunLoop 是对 CFRunLoop 的简单封装，需要着重研究的只有 CFRunLoop

更准确的说，代码里面是 `CFRunLoopRef`，本文统一简称为 `CFRunLoop`

# RunLoop 与线程的关系
苹果不允许直接创建 RunLoop，它只提供了两个自动获取的函数：

Core Foundation 框架中的获取方法：CFRunLoopGetMain() 和 CFRunLoopGetCurrent()

Foundation 框架中的获取方法：[NSRunLoop mainRunLoop] 和 [NSRunLoop currentRunLoop]

代码逻辑如下：

```c
/// 全局的Dictionary，key 是 pthread_t， value 是 CFRunLoopRef
static CFMutableDictionaryRef loopsDic;
/// 访问 loopsDic 时的锁
static CFSpinLock_t loopsLock;
 
/// 获取一个 pthread 对应的 RunLoop。
CFRunLoopRef _CFRunLoopGet(pthread_t thread) {
    OSSpinLockLock(&loopsLock);
    
    if (!loopsDic) {
        // 第一次进入时，初始化全局Dic，并先为主线程创建一个 RunLoop。
        loopsDic = CFDictionaryCreateMutable();
        CFRunLoopRef mainLoop = _CFRunLoopCreate();
        CFDictionarySetValue(loopsDic, pthread_main_thread_np(), mainLoop);
    }
    
    /// 直接从 Dictionary 里获取。
    CFRunLoopRef loop = CFDictionaryGetValue(loopsDic, thread));
    
    if (!loop) {
        /// 取不到时，创建一个
        loop = _CFRunLoopCreate();
        CFDictionarySetValue(loopsDic, thread, loop);
        /// 注册一个回调，当线程销毁时，顺便也销毁其对应的 RunLoop。
        _CFSetTSD(..., thread, loop, __CFFinalizeRunLoop);
    }
    
    OSSpinLockUnLock(&loopsLock);
    return loop;
}
 
CFRunLoopRef CFRunLoopGetMain() {
    return _CFRunLoopGet(pthread_main_thread_np());
}
 
CFRunLoopRef CFRunLoopGetCurrent() {
    return _CFRunLoopGet(pthread_self());
}
```

从上面的代码可以看出

+ 线程和 RunLoop 之间是一一对应的，其关系是保存在一个全局的 Dictionary 里，但是不代表有线程就有 RunLoop

+ 除了主线程，如果想要获取线程的 RunLoop，只能在当前线程内获取；

+ 除了主线程，如果没有主动获取线程的 RunLoop，则 RunLoop 不会创建

+ RunLoop 会在线程销毁时销毁

+ CFRunLoop 是基于 pthread 来管理的

> pthread_t 和 NSThread 是一一对应的
> 
> 可以通过 `pthread_main_thread_np()` 或 [NSThread mainThread] 来获取主线程；
> 
> 也可以通过 `pthread_self()` 或 [NSThread currentThread] 来获取当前线程。

# RunLoop 的结构
## 结构图

![](https://blog.ibireme.com/wp-content/uploads/2015/05/RunLoop_0.png)

在 CoreFoundation 里面关于 RunLoop 有5个类：

+ CFRunLoopRef
+ CFRunLoopModeRef
+ CFRunLoopSourceRef
+ CFRunLoopTimerRef
+ CFRunLoopObserverRef

一个 RunLoop 包含若干个 Mode，每个 Mode 又包含若干个 Source/Timer/Observer（这三个都被称为 Mode Item）。每次调用 RunLoop 的主函数时，只能指定其中一个 Mode，这个 Mode被称作 CurrentMode。如果需要切换 Mode，只能退出 Loop，再重新指定一个 Mode 进入。这样做主要是为了分隔开不同组的 Source/Timer/Observer，让其互不影响

每次 RunLoop 只会以一种 Mode 运行，以该 Mode 运行的时候，就只执行和该 Mode 相关的任务，只通知该 Mode 注册过的 Observer

## Mode Item
Source/Timer/Observer 都被称为 mode item，一个 item 可以被同时加入多个 mode。但一个 item 被重复加入同一个 mode 时是不会有效果的

### CFRunLoopSource
CFRunLoopSource 是事件产生的地方。

Source有两个版本：Source0 和 Source1

+ Source0 只包含了一个回调（函数指针），它并不能主动触发事件。使用时，你需要先调用 CFRunLoopSourceSignal(source)，将这个 Source 标记为待处理，然后手动调用 CFRunLoopWakeUp(runloop) 来唤醒 RunLoop，让其处理这个事件
+ Source1 是基于 port 的，包含了一个 mach_port 和一个回调（函数指针），可以接收内核消息并触发回调。这种 Source 能主动唤醒 RunLoop 的线程，比如触摸/锁屏/摇晃/点击

### CFRunLoopTimer
CFRunLoopTimer 是基于时间的触发器，它和 NSTimer 是 toll-free bridged 的，可以混用。其包含一个时间长度和一个回调（函数指针）。当其加入到 RunLoop 时，RunLoop 会注册对应的时间点，当时间点到时，RunLoop 会被唤醒以执行那个回调

### CFRunLoopObserver
CFRunLoopObserver 是观察者，每个 Observer 都包含了一个回调（函数指针），当 RunLoop 的状态发生变化时，观察者就能通过回调接受到这个变化。可以观测的时间点有以下几个：

```c
typedef CF_OPTIONS(CFOptionFlags, CFRunLoopActivity) {
    kCFRunLoopEntry         = (1UL << 0), // 即将进入Loop
    kCFRunLoopBeforeTimers  = (1UL << 1), // 即将处理 Timer
    kCFRunLoopBeforeSources = (1UL << 2), // 即将处理 Source
    kCFRunLoopBeforeWaiting = (1UL << 5), // 即将进入休眠
    kCFRunLoopAfterWaiting  = (1UL << 6), // 刚从休眠中唤醒
    kCFRunLoopExit          = (1UL << 7), // 即将退出Loop
};
```

## 事件源
![](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Multithreading/Art/runloop.jpg)

### 种类
图中展现了 RunLoop 在线程中的作用：从 input source 和 timer source 接受事件，然后在线程中处理事件

Run Loop 的处理两大类事件源：

+ Timer Source
+ Input Source
    + performSelector 的方法簇
    + Port
    + 自定义的 Input Source

### 事件源缺失的后果
如果一个 mode 没有 Source0/Source1/Timer（不管有没有 Observer），则 RunLoop 会直接退出，不进入循环。详搜 `__CFRunLoopModeIsEmpty`

# RunLoop Mode
## 种类
苹果公开提供的 Mode 有两个：kCFRunLoopDefaultMode (NSDefaultRunLoopMode) 和 UITrackingRunLoopMode

+ NSDefaultRunLoopMode(kCFRunLoopDefaultMode)
+ NSConnectionReplyMode
+ NSModalPanelRunLoopMode
+ NSEventTrackingRunLoopMode(UITrackingRunLoopMode)
+ NSRunLoopCommonModes(kCFRunLoopCommonModes)

其中比较重要的模式如下，其他 Mode 不需要管

NSDefaultRunLoopMode：App的默认 Mode，通常主线程是在这个 Mode 下运行的

UITrackingRunLoopMode：界面跟踪 Mode，用于 ScrollView 追踪触摸滑动，保证界面滑动时不受其他 Mode 影响

NSRunLoopCommonModes：实际上是一个 Mode 的集合，默认包括 NSDefaultRunLoopMode 和 UITrackingRunLoopMode。可以用这个字符串来操作 Common Items，或标记一个 Mode 为 "Common"



## Common Modes
```c
struct __CFRunLoop {
    CFMutableSetRef _commonModes;     // Set
    CFMutableSetRef _commonModeItems; // Set<Source/Observer/Timer>
    CFRunLoopModeRef _currentMode;    // Current Runloop Mode
    CFMutableSetRef _modes;           // Set
    ...
};

struct __CFRunLoopMode {
    CFStringRef _name;            // Mode Name, 例如 @"kCFRunLoopDefaultMode"
    CFMutableSetRef _sources0;    // Set
    CFMutableSetRef _sources1;    // Set
    CFMutableArrayRef _observers; // Array
    CFMutableArrayRef _timers;    // Array
    ...
};
```

其中 `_commonModes` 其实并不是一个真正的模式，可以看到它是 Modes 而不是 Mode，是一个模式的集合

一个 Mode 可以将自己标记为 "Common" 属性，对应的方法是 `CFRunLoopAddCommonMode(CFRunLoopRef runloop, CFStringRef modeName);`

添加 Source/Observer/Timer 时，如果指定的模式为 kCFRunLoopCommonModes，则会被添加 `_commonModeItems` 中；每当 RunLoop 的内容发生变化时，RunLoop 都会自动将 `_commonModeItems` 里的 Source/Observer/Timer 同步到具有 "Common" 标记的所有 Mode 里

kCFRunLoopDefaultMode 默认是 "Common" 的

# RunLoop 内部逻辑
![](http://ww1.sinaimg.cn/large/99e3e31egy1gb0wa37px1j20ou1hz770.jpg)

[流程图源文件](https://www.processon.com/view/link/5d22043ce4b0fdb331d5a8ba)

[对应源码](https://opensource.apple.com/source/CF/CF-1153.18/CFRunLoop.c.auto.html)

其中 poll = 处理了 source0 || 没有超时

此处最好看源码！！！

# RunLoop 的底层实现
RunLoop 进入休眠时调用的函数是 `mach_msg()`，实际上是调用了一个 Mach 陷阱 `mach_msg_trap()`，当你在用户态调用 `mach_msg_trap()` 时会触发陷阱机制，切换到内核态；内核态中内核实现的 `mach_msg()` 函数会休眠并监听端口等待唤醒

休眠的具体流程如下：

1. 指定一个将来唤醒自己的`mach_port`端口

2. 调用 `mach_msg` 来监听这个端口，保持`mach_msg_trap`状态

3. 由另一个线程（比如有可能有一个专门处理键盘输入事件的 loop 在后台一直运行）向内核发送这个端口的msg后，`mach_msg_trap` 状态被唤醒，RunLoop 继续运行

![](https://blog.ibireme.com/wp-content/uploads/2015/05/RunLoop_5.png)

# RunLoop 的应用
## 系统应用
### 事件响应（重要）
如果发生触摸/锁屏/摇晃/点击等事件，首先是由 Source1 接收 IOHIDEvent，唤醒 RunLoop；之后在 Source1 的回调 `__IOHIDEventSystemClientQueueCallback()` 内触发 Source0 回调，Source0 的回调内部调用 UIApplication 将事件封装为 UIEvent 并分发出去。所以 UIButton 的点击事件在堆栈中看到是在 Source0 内的

### 界面更新（重要）
setNeedsLayout/setNeedsDisplay方法后，这个 UIView/CALayer 就被标记为待处理，并被提交到一个全局的容器去

苹果注册了一个 Observer 监听 BeforeWaiting(即将进入休眠) 和 Exit (即将退出Loop) 事件，回调执行一个函数：遍历所有待处理的 UIView/CAlayer 以执行实际的绘制和调整，并更新 UI 界面

### 定时器（重要）
一个 NSTimer 注册到 RunLoop 后，RunLoop 会为其重复的时间点注册好事件。例如 10:00, 10:10, 10:20 这几个时间点。RunLoop 为了节省资源，并不会在非常准确的时间点回调这个Timer。Timer 有个属性叫做 Tolerance (宽容度)，标示了当时间点到后，容许有多少最大误差。

由于 NSTimer 的这种机制，因此 NSTimer 的执行必须依赖于 RunLoop，如果没有 RunLoop，NSTimer 是不会执行的

如果某个时间点被错过了，例如执行了一个很长的任务，则那个时间点的回调也会跳过去，不会延后执行

CADisplayLink 是一个和屏幕刷新率一致的定时器，比 NSTimer 精度更高。如果在两次屏幕刷新之间执行了一个长任务，那其中就会有一帧被跳过去（和 NSTimer 相似），造成界面卡顿的感觉。通常情况下 CADisaplayLink 用于构建帧动画，看起来相对更加流畅，而 NSTimer 则有更广泛的用处

### AutoreleasePool（重要）
+ 进入之前，创建 AutoreleasePool（监听 kCFRunLoopEntry）
+ 休眠之前，销毁当前再创建一个新的 AutoreleasePool（监听 kCFRunLoopBeforeWaiting）
+ 退出之前，销毁 AutoreleasePool（监听 kCFRunLoopExit）

## RunLoop 的实践应用
### 卡顿检测（重要）
RunLoop 处理事件的时间主要出在两个阶段：

1. kCFRunLoopBeforeSources 和 kCFRunLoopBeforeWaiting 之间
2. kCFRunLoopAfterWaiting 之后

我们可以向主线程注册 Observer 观察其 RunLoop 的回调，如果回调时间过长，则认为发生卡顿

这个例子中，RunLoop 的回调用来发送信号量（就像喂食）；子线程则不断等待 RunLoop 发送的信号量并消耗它，最多等待 5 秒（等吃，如果有则吃掉，如果等了太久则不等了，开始哭）；超时发生时，一般 lastActivity 是 kCFRunLoopBeforeSources or kCFRunLoopAfterWaiting（哭一般是因为等待着两个事件）

```objc
@interface RunLoopMonitor()
{
    CFRunLoopObserverRef _observer;  // 观察者
    dispatch_semaphore_t _semaphore; // 信号量
    CFRunLoopActivity _lastActivity;     // 状态
}
@end

@implementation RunLoopMonitor
+ (instancetype)sharedInstance
{
    static id sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
        // 开始监听
        [self registerObserver];
    });
    return sharedInstance;
}

static void runLoopObserverCallBack(CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info)
{
    RunLoopMonitor *instance = [RunLoopMonitor sharedInstance];
    // 记录状态值
    instance-> _lastActivity = activity;
    // 发送信号
    dispatch_semaphore_signal(instance->_semaphore);
}

// 注册一个Observer来监测Loop的状态,回调函数是runLoopObserverCallBack
- (void)registerObserver
{
    // 这是在主线程
    // 设置Runloop observer的运行环境
    CFRunLoopObserverContext context = {0, (__bridge void *)self, NULL, NULL};
    // 创建Runloop observer对象，监听所有的状态
    _observer = CFRunLoopObserverCreate(kCFAllocatorDefault,
                                        kCFRunLoopAllActivities,
                                        YES,
                                        0,
                                        &runLoopObserverCallBack,
                                        &context);
    // 将新建的observer加入到当前thread的runloop
    CFRunLoopAddObserver(CFRunLoopGetMain(), _observer, kCFRunLoopCommonModes);
    // 创建信号，初识信号量为 0
    _semaphore = dispatch_semaphore_create(0);
    // 子线程监控时长
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
    	 // 这里的 while 可以改进为创建一个常驻子线程
        while (YES)
        {
            // dispatch_semaphore_wait：
            // 如果信号量的值大于0，该函数所处线程就继续执行下面的语句，并且将信号量的值减1；
            // 如果为0，那么这个函数就阻塞当前线程等待 timeout
            // 如果等待期间信号量被 dispatch_semaphore_signal 加1了那么就继续向下执行并将信号量减1。
            // 如果等待期间没有获取到信号量或者信号量的值一直为0，那么等到timeout时，其所处线程自动执行其后语句
            // 返回值为 0 则表示在规定时间内等到了
            // 返回值不为 0 则代表在规定时间内也没收到信号，超时了
            long ret = dispatch_semaphore_wait(self->_semaphore, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_MSEC));
            if (ret != 0)
            {
                if (self-> _lastActivity == kCFRunLoopBeforeSources ||
                    self-> _lastActivity == kCFRunLoopAfterWaiting)
                {
                    NSLog(@"超过 5 秒没有收到信号了");
                }
            }
        }
    });
}
@end
```

+ [RunLoop实战：实时卡顿监控](https://juejin.im/post/5cacb2baf265da03904bf93b)
+ [关于dispatch_semaphore的使用](https://www.cnblogs.com/snailHL/p/3906112.html)

### 滚动时延迟加载图片？（重要）
当设置图片的时候，让其在 CFRunLoopDefaultMode 下进行

```objc
[self.imageView performSelector:@selector(setImage:) withObject:[UIImage imageNamed:@"imgName"] afterDelay:3.0 inModes:@[NSDefaultRunLoopMode]];
```

上面的代码可以达到如下效果：
用户点击屏幕，在主线程中，三秒之后显示图片。但是当用户点击屏幕之后，如果此时用户开始滚动，那么就算过了三秒，图片也不会显示出来，当停止滚动才会显示图片。

这是因为 setImage 只能在 NSDefaultRunLoopMode 模式下使用，当滚动 tableView 的时候，RunLoop 是在 UITrackingRunLoopMode 这个 Mode 下，就不会设置图片，当停止的时候才切回 NSDefaultRunLoopMode

另一个例子，怎样保证子线程数据回来更新UI的时候不打断用户的滑动操作？

当我们在子请求数据的同时滑动浏览当前页面，如果数据请求成功要切回主线程更新UI，那么就会影响当前正在滑动的体验。
我们就可以将更新UI事件放在主线程的 NSDefaultRunLoopMode 上执行即可，这样就会等用户不再滑动页面，主线程 RunLoop 由 UITrackingRunLoopMode 切换到 NSDefaultRunLoopMode 时再去更新UI

```objc
[self performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO modes:@[NSDefaultRunLoopMode]];
```

但是这里的问题是 modes 不指定的时候，就是 DefaultMode 呀，[引用官方文档](https://developer.apple.com/documentation/objectivec/nsobject/1416176-performselector?language=objc) 

> This method sets up a timer to perform the aSelector message on the current thread’s run loop. The timer is configured to run in the default mode (NSDefaultRunLoopMode)

### Timer 滚动没回调（重要）
NSTimer 在 ScrollView 滚动的时候没有回调，如何解决呢

主线程的 RunLoop 里有两个预置的 Mode：kCFRunLoopDefaultMode 和 UITrackingRunLoopMode。这两个 Mode 默认都是 Common Mode

DefaultMode 是 App 平时所处的状态，TrackingRunLoopMode 是追踪 ScrollView 滑动时的状态

当你创建一个 Timer，默认是被加到 DefaultMode，Timer 正常情况下会得到重复回调，但此时滑动一个 ScrollView 时，RunLoop 会将 mode 切换为 TrackingRunLoopMode，这时 Timer 就不会被回调。因为 RunLoop 运行时只能指定一个 Mode。如果需要切换 Mode，只能退出 Loop，再重新指定一个 Mode 进入。TrackingRunLoopMode 下，处在 DefaultMode 的 Timer 是不会被通知到的

解决方法是将 Timer 加到 CommonModes 中去

```objc
// 这种创建方法默认不会加到任何 RunLoop
NSTimer *timer = [NSTimer timerWithTimeInterval:1.0 target:self selector:@selector(run) userInfo:nil repeats:YES];

// 如果这样写，滚动时 NSTimer 不会回调
// [[NSRunLoop mainRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];

// 滚动时 NSTimer 会回调
// 解决方法 1：添加到 CommonModes
[[NSRunLoop mainRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
// 解决方法 2：手动添加到 TrackingMode
[[NSRunLoop mainRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
[[NSRunLoop mainRunLoop] addTimer:timer forMode:UITrackingRunLoopMode];
```

也可以使用 GCD 定时器，它不会受 RunLoop 的影响

### PerformSelector && 子线程启动 Timer 失效（重要）
当调用 NSObject 的 performSelector:afterDelay: 后，实际上其内部会创建一个 Timer 并添加到当前线程的 RunLoop 中。所以如果当前线程没有 RunLoop，则这个方法会失效

当调用 performSelector:onThread: 时，实际上其会创建一个 Timer 加到对应的线程去，同样的，如果对应线程没有 RunLoop 该方法也会失效

```objc
dispatch_async(dispatch_get_global_queue(0, 0), ^{
    // 如果只写以下一行代码不会生效，本质是生成一个 Timer 添加到 RunLoop
    [self performSelector:@selector(run) withObject:nil afterDelay:0];
    // 必须手动开启 RunLoop Timer 才能生效
    [[NSRunLoop currentRunLoop] run];
});
```

子线程的 RunLoop 默认不开启，必须手动开启

```objc
dispatch_queue_t queue = dispatch_queue_create("test", DISPATCH_QUEUE_SERIAL);
// 在子线程中使用定时器
dispatch_async(queue, ^{
    // 第一种方式
    // 创建的 timer 已经添加至当前的 runloop 中
    [NSTimer scheduledTimerWithTimeInterval:2.0 target:self selector:@selector(doSomething) userInfo:nil repeats:YES];
    // 在线程中使用定时器，如果不启动run loop，timer的事件是不会响应的，而子线程中runloop默认没有启动
    // 让线程执行一个周期性的任务，如果不启动run loop， 线程跑完就可能被系统释放了
    [[NSRunLoop currentRunLoop] run];// 如果没有这句，doSomething将不会执行！！！

    /*************************************************************/

    // 第二种方式
    // 创建的 timer 没有默认添加到 runloop 中
	NSTimer *timer = [NSTimer timerWithTimeInterval:2.0 target:self selector:@selector(doSomething) userInfo:nil repeats:NO];
    // 将定时器添加到runloop中
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
    // 在线程中使用定时器，如果不启动run loop，timer的事件是不会响应的，而子线程中runloop默认没有启动
    // 让线程执行一个周期性的任务，如果不启动run loop， 线程跑完就可能被系统释放了
    [[NSRunLoop currentRunLoop] run];// 如果没有这句，doSomething将不会执行！！！
});
```

### 子线程常驻（重要）
当我们使用 GCD 的方法创建了子线程，那么当子线程中的任务执行完毕后，子线程就会被销毁掉

如果我们需要经常在子线程中执行此任务，只使用 GCD 会导致线程的频繁创建和销毁，此时我们就需要保证一个子线程的常驻

```objc
dispatch_async(dispatch_get_global_queue(0, 0), ^{
	// 通过访问 RunLoop 来创建子线程的一个 RunLoop
	NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
	// 向该RunLoop中添加一个Port/Source等维持RunLoop的事件循环
	[runLoop addPort:[NSMachPort port] forMode:NSDefaultRunLoopMode];
	// 启动 RunLoop
	[runLoop run];
};
```

此处添加 port 只是为了让 RunLoop 不至于退出，并没有用于实际的发送消息

以下是 AFNetWorking 的 RunLoop 示例代码

```objc
+ (void)networkRequestThreadEntryPoint:(id)__unused object {
    @autoreleasepool {
        [[NSThread currentThread] setName:@"AFNetworking"];
        NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
        [runLoop addPort:[NSMachPort port] forMode:NSDefaultRunLoopMode];
        [runLoop run];
    }
}
 
+ (NSThread *)networkRequestThread {
    static NSThread *_networkRequestThread = nil;
    static dispatch_once_t oncePredicate;
    dispatch_once(&oncePredicate, ^{
        _networkRequestThread = [[NSThread alloc] initWithTarget:self selector:@selector(networkRequestThreadEntryPoint:) object:nil];
        [_networkRequestThread start];
    });
    return _networkRequestThread;
}
```


# 深度好文
[我的脑图](http://naotu.baidu.com/file/1048175c83ab832d33e9650c84ef2abe?token=70060f6e559654ce)

[深入理解RunLoop](https://blog.ibireme.com/2015/05/18/runloop/)

[iOS RunLoop详解](https://juejin.im/post/5aca2b0a6fb9a028d700e1f8)

[RunLoop 源码剖析](https://github.com/Desgard/iOS-Source-Probe/blob/master/Objective-C/Foundation/Run%20Loop%20%E8%AE%B0%E5%BD%95%E4%B8%8E%E6%BA%90%E7%A0%81%E6%B3%A8%E9%87%8A.md)

[关于runloop，好多人都理解错了！](https://www.jianshu.com/p/ae0118f968bf)

[RunLoop 源码](https://opensource.apple.com/source/CF/CF-855.17/CFRunLoop.c.auto.html)