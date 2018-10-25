# NSRunLoop 和 CFRunLoopRef
OSX/iOS 系统中，提供了两个这样的对象：NSRunLoop 和 CFRunLoop

CFRunLoop 是 Core Foundation 框架内的，它提供了纯 C 函数的 API，所有这些 API 都是线程安全的

NSRunLoop 是 Foundation 框架内的，提供了面向对象的 API，但是这些 API 不是线程安全的

其中 NSRunLoop 是对 CFRunLoop 的简单封装，需要着重研究的只有 CFRunLoop

# RunLoop 与线程的关系
## pthread_t 和 NSThread
pthread_t 和 NSThread 是一一对应的

可以通过 `pthread_main_thread_np()` 或 [NSThread mainThread] 来获取主线程；也可以通过 `pthread_self()` 或 [NSThread currentThread] 来获取当前线程。

CFRunLoop 是基于 pthread 来管理的

## RunLoop 与线程
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


# RunLoop 的结构
## 结构图与代码

![](https://blog.ibireme.com/wp-content/uploads/2015/05/RunLoop_0.png)

一个 RunLoop 包含若干个 Mode，每个 Mode 又包含若干个 Source/Timer/Observer。每次调用 RunLoop 的主函数时，只能指定其中一个 Mode，这个Mode被称作 CurrentMode。如果需要切换 Mode，只能退出 Loop，再重新指定一个 Mode 进入。这样做主要是为了分隔开不同组的 Source/Timer/Observer，让其互不影响

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

## CFRunLoopSource
CFRunLoopSource 是事件产生的地方。

Source有两个版本：Source0 和 Source1

+ Source0 只包含了一个回调（函数指针），它并不能主动触发事件。使用时，你需要先调用 CFRunLoopSourceSignal(source)，将这个 Source 标记为待处理，然后手动调用 CFRunLoopWakeUp(runloop) 来唤醒 RunLoop，让其处理这个事件
+ Source1 是基于 port 的，包含了一个 mach_port 和一个回调（函数指针），被用于通过内核和其他线程相互发送消息。这种 Source 能主动唤醒 RunLoop 的线程，比如触摸/锁屏/摇晃/点击

## CFRunLoopTimer
CFRunLoopTimer 是基于时间的触发器，它和 NSTimer 是 toll-free bridged 的，可以混用。其包含一个时间长度和一个回调（函数指针）。当其加入到 RunLoop 时，RunLoop会注册对应的时间点，当时间点到时，RunLoop会被唤醒以执行那个回调

## CFRunLoopObserver
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
### 事件源缺失的后果
上面的 Source/Timer/Observer 被统称为 mode item，一个 item 可以被同时加入多个 mode。但一个 item 被重复加入同一个 mode 时是不会有效果的。

![](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Multithreading/Art/runloop.jpg)

图中展现了 RunLoop 在线程中的作用：从 input source 和 timer source 接受事件，然后在线程中处理事件

Run Loop 的处理两大类事件源：

+ Timer Source
+ Input Source
    + performSelector 的方法簇
    + Port
    + 自定义的Input Source

如果一个 mode 中没有 Source0/Source1/Timer（不用管有没有 Observer），则 RunLoop 会直接退出，不进入循环。详搜 `__CFRunLoopModeIsEmpty`

### 如何保持一个子线程的 RunLoop

```objc
// 通过访问 RunLoop 来创建子线程的一个 RunLoop
NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
// 向该RunLoop中添加一个Port/Source等维持RunLoop的事件循环
[runLoop addPort:[NSMachPort port] forMode:NSDefaultRunLoopMode];
// 启动 RunLoop
[runLoop run];
```





# Common Mode
## 特点
kCFRunLoopCommonModes 其实并不是一个真正的模式，可以看到它是 Modes 而不是 Mode，是一个模式的集合

一个 Mode 可以将自己标记为"Common"属性，即 `CFRunLoopAddCommonMode(CFRunLoopRef runloop, CFStringRef modeName);`

每当 RunLoop 的内容发生变化时，RunLoop 都会自动将 _commonModeItems 里的 Source/Observer/Timer 同步到具有 "Common" 标记的所有Mode里。

## Timer 与 RunLoop
NSTimer 在 ScrollView 滚动的时候没有回调，如何解决呢

主线程的 RunLoop 里有两个预置的 Mode：kCFRunLoopDefaultMode 和 UITrackingRunLoopMode。这两个 Mode 都是 Common Mode

DefaultMode 是 App 平时所处的状态，TrackingRunLoopMode 是追踪 ScrollView 滑动时的状态。

当你创建一个 Timer 并加到 DefaultMode 时，Timer 正常情况下会得到重复回调，但此时滑动一个 ScrollView 时，RunLoop 会将 mode 切换为 TrackingRunLoopMode，这时 Timer 就不会被回调。因为 RunLoop 运行时只能指定一个 Mode。如果需要切换 Mode，只能退出 Loop，再重新指定一个 Mode 进入。TrackingRunLoopMode 下，处在 DefaultMode 的 Timer 是不会被通知到的

解决方法是将 Timer 加到 CommonModes 中去

```objc
// 滚动时 NSTimer 不会回调
[[NSRunLoop mainRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];

// 滚动时 NSTimer 会回调
[[NSRunLoop mainRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
```

# RunLoop 事件循环机制
![](https://blog.ibireme.com/wp-content/uploads/2015/05/RunLoop_1.png)

这个图有 2 处错误：

1. 第五步改为：如果在主线程，检查是否有 GCD 事件需要处理，有的话，跳转到第 9 步
2. 第七步，Source0(port) 改为 Source1(port)

以下是根据最新的 [CFRunLoop.c](https://opensource.apple.com/source/CF/CF-1153.18/CFRunLoop.c.auto.html) 整理的运行步骤：
>
1. 通知 observers: kCFRunLoopEntry, 进入 run loop
2. 通知 observers: kCFRunLoopBeforeTimers, 即将处理 timers
3. 通知 observers: kCFRunLoopBeforeSources, 即将处理 sources
4. 处理 blocks
5. 处理 sources 0
6. 如果第 5 步实际处理了 sources 0，再一次处理 blocks
7. 如果在主线程，检查是否有 GCD 事件需要处理，有的话，跳转到第 11 步
8. 通知 observers: kCFRunLoopBeforeWaiting, 即将进入等待（睡眠）（如果处理了 source0，不会通知睡眠、进入睡眠、通知唤醒）
9. 等待被唤醒，可以被 sources1、timers、外部手动（CFRunLoopWakeUp 函数和 GCD 事件（如果在主线程））唤醒
10. 通知 observers: kCFRunLoopAfterWaiting, 即停止等待（被唤醒）
11. 被什么唤醒就处理什么：
    + 被 timers 唤醒，处理 timers
    + 被 GCD 唤醒或者从第 7 步跳转过来的话，处理 GCD
    + 被 sources 1 唤醒，处理 sources 1
12. 再一次处理 blocks
13. 判断是否退出，不需要退出则跳转回第 2 步
14. 通知 observers: kCFRunLoopExit, 退出 run loop


有一点出入的地方是如果在第 5 步实际处理了 sources 0，是不会进入睡眠的。具体可以看源码

# RunLoop 的底层实现
RunLoop 进入休眠时调用的函数是 `mach_msg()`，实际上是调用了一个 Mach 陷阱 `mach_msg_trap()`，当你在用户态调用 `mach_msg_trap()` 时会触发陷阱机制，切换到内核态；内核态中内核实现的 `mach_msg()` 函数会休眠并监听端口等待唤醒

休眠的具体流程如下：

1. 指定一个将来唤醒自己的`mach_port`端口

2. 调用mach_msg来监听这个端口，保持`mach_msg_trap`状态

3. 由另一个线程（比如有可能有一个专门处理键盘输入事件的loop在后台一直运行）向内核发送这个端口的msg后，`mach_msg_trap`状态被唤醒，RunLoop继续运行

![](https://blog.ibireme.com/wp-content/uploads/2015/05/RunLoop_5.png)

# RunLoop 的系统应用

## AutoreleasePool
## 事件响应
如果发生触摸/锁屏/摇晃/点击等事件，首先是由 Source1 接收 IOHIDEvent，唤醒 RunLoop；之后在 Source1 的回调 `__IOHIDEventSystemClientQueueCallback()` 内触发 Source0 回调，Source0 的回调内部调用 UIApplication 将事件封装为 UIEvent 并分发出去。所以 UIButton 的点击事件在堆栈中看到是在 Source0 内的

![](http://qingmo.me/images/uitouchflow.png)
![](http://qingmo.me/images/calltraceoftouching.png)

## 手势识别
## 界面更新
setNeedsLayout/setNeedsDisplay方法后，这个 UIView/CALayer 就被标记为待处理，并被提交到一个全局的容器去。

苹果注册了一个 Observer 监听 BeforeWaiting(即将进入休眠) 和 Exit (即将退出Loop) 事件，回调执行一个函数：遍历所有待处理的 UIView/CAlayer 以执行实际的绘制和调整，并更新 UI 界面。
## 定时器
## PerformSelecter
当调用 NSObject 的 performSelecter:afterDelay: 后，实际上其内部会创建一个 Timer 并添加到当前线程的 RunLoop 中。所以如果当前线程没有 RunLoop，则这个方法会失效。

当调用 performSelector:onThread: 时，实际上其会创建一个 Timer 加到对应的线程去，同样的，如果对应线程没有 RunLoop 该方法也会失效。
## GCD
## NSURLConnection

# RunLoop 的实际应用
## AFNetworking
## AsyncDisplayKit
## 卡顿检测
在主线程的 RunLoop 中添加一个 observer，检测从 kCFRunLoopBeforeSources 到 kCFRunLoopBeforeWaiting 花费的时间是否过长。

如果花费的时间大于某一个阙值，我们就认为有卡顿
## TableView延迟加载图片
当设置图片的时候，让其在 CFRunLoopDefaultMode 下进行。

当滚动 tableView 的时候，RunLoop 是在 UITrackingRunLoopMode 这个 Mode 下，就不会设置图片，当停止的时候，就会设置图片

# 深度好文
[深入理解RunLoop](https://blog.ibireme.com/2015/05/18/runloop/)
[RunLoop 源码剖析](https://github.com/Desgard/iOS-Source-Probe/blob/master/Objective-C/Foundation/Run%20Loop%20%E8%AE%B0%E5%BD%95%E4%B8%8E%E6%BA%90%E7%A0%81%E6%B3%A8%E9%87%8A.md)
[关于runloop，好多人都理解错了！](https://www.jianshu.com/p/ae0118f968bf)
[iOS触摸事件的流动](http://qingmo.me/2017/03/04/FlowOfUITouch/)
