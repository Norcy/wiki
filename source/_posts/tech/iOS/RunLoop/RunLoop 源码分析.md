[本文对应的源码地址——CFRunLoop.h](https://opensource.apple.com/source/CF/CF-1153.18/CFRunLoop.h.auto.html)
[本文对应的源码地址——CFRunLoop.c](https://opensource.apple.com/source/CF/CF-1153.18/CFRunLoop.c.auto.html)


## 数据结构
### CFRunLoop
```c
struct __CFRunLoop {
    pthread_t _pthread;                 // RunLoop 对应的线程
    CFMutableSetRef _commonModes;       // CommonMode 的集合
    CFMutableSetRef _commonModeItems;   // CommondModeItem 的集合
    CFRunLoopModeRef _currentMode;      // 当前的 Mode
    CFMutableSetRef _modes;             // 该 RunLoop 包含的 Mode
    ...
};
```

1. RunLoop 与线程一一对应
2. CommonModes 是一个集合，CommondModeItem 也是一个集合
3. 虽然 RunLoop 可以包含多个 Modes，但是 currentMode 只能有一个，即在同一个时间只能指定一种 Mode 运行

### CFRunLoopMode
```c
typedef struct __CFRunLoopMode *CFRunLoopModeRef;

struct __CFRunLoopMode {
    CFStringRef _name;              // Mode 的名称
    CFMutableSetRef _sources0;      // Source0 集合
    CFMutableSetRef _sources1;      // Source1 集合
    CFMutableArrayRef _observers;   // Observer 数组
    CFMutableArrayRef _timers;      // Timer 数组
    ...
};
```

1. CFRunLoopModeRef 是指向 `__CFRunLoopMode` 的指针，我们只需要研究 `__CFRunLoopMode` 即可
2. CFRunLoop 管理了 CFRunLoopModeRef 的集合
3. CFRunLoopModeRef 包含了 Source0/Source1 的集合；以及 Observer/Timer 的数组（为什么 Source 是集合，Observer/Timer 是数组？）
4. Source0/Source1 的类型是 CFRunLoopSource（虽然上面代码看不出来）

### CFRunLoopSource
```c
struct __CFRunLoopSource {
    CFMutableBagRef _runLoops;              // 一个 Source 可以被加入到多个 RunLoop
    union {
        CFRunLoopSourceContext version0;    // Source0
        CFRunLoopSourceContext1 version1;   // Source1
    } _context;
};

typedef struct {
    CFIndex version;        // 区分是 Source0 还是 Source1
    void    (*schedule)(void *info, CFRunLoopRef rl, CFStringRef mode);
    void    (*cancel)(void *info, CFRunLoopRef rl, CFStringRef mode);
    void    (*perform)(void *info);
    ...
} CFRunLoopSourceContext;

typedef struct {
    CFIndex version;        // 区分是 Source0 还是 Source1
    mach_port_t (*getPort)(void *info);     // Source1 是基于 Port
    void *  (*perform)(void *msg, CFIndex size, CFAllocatorRef allocator, void *info);
    ...
} CFRunLoopSourceContext1;
```

1. 一个 Source 可以被加入到多个 RunLoop
2. CFRunLoopSource 包含了 Source0/Source1，union 中所有成员变量的起始地址都是一样的，Source0 和 Source 共占同一段内存的结构，`_context` 变量的大小取决于 Source0 和 Source1 的最大大小
3. Source0 比 Source1 多了 schedule 和 cancle 方法？；而 Source1 比 Source0 多了接受 Port 消息的方法。所以说 Source1 是基于 Port 的

### CFRunLoopObserver
```c
struct __CFRunLoopObserver {
    CFRunLoopRef _runLoop;          // Observer 所在的 RunLoop
    CFIndex _rlCount;               // Observer 当前监测的 RunLoop 数
    CFOptionFlags _activities;      // Observer 可以回调给监听者的时间点
    ...
};
```

1. 一个 Observer 只能监听一个 RunLoop

2. 其中 rlCount 是用来更新 runloop 的值（如果 Observer 添加到 A，B，再从 A 中移除，此时是有问题的，Observer 的 runloop 仍然指向 A？）
	
	```c
	static void __CFRunLoopObserverSchedule(CFRunLoopObserverRef rlo, CFRunLoopRef rl, CFRunLoopModeRef rlm) {
	    __CFRunLoopObserverLock(rlo);
	    // 如果一个 Observer 被添加到多个 RunLoop，则只有第一个会生效
	    if (0 == rlo->_rlCount) {
	        rlo->_runLoop = rl;
	    }
	    rlo->_rlCount++;
	    __CFRunLoopObserverUnlock(rlo);
	}
	
	static void __CFRunLoopObserverCancel(CFRunLoopObserverRef rlo, CFRunLoopRef rl, CFRunLoopModeRef rlm) {
	    __CFRunLoopObserverLock(rlo);
	    rlo->_rlCount--;
	    // 如果一个 Observer 没有监听任何 RunLoop，则重置 _runLoop
	    if (0 == rlo->_rlCount) {
	        rlo->_runLoop = NULL;
	    }
	    __CFRunLoopObserverUnlock(rlo);
	}
	```

3. CFOptionFlags 是枚举值，包含以下时间点

	```c
	typedef CF_OPTIONS(CFOptionFlags, CFRunLoopActivity) {
	    kCFRunLoopEntry = (1UL << 0),           // 开始进入
	    kCFRunLoopBeforeTimers = (1UL << 1),    // 即将处理 Timer
	    kCFRunLoopBeforeSources = (1UL << 2),   // 即将处理 Source
	    kCFRunLoopBeforeWaiting = (1UL << 5),   // 即将休眠
	    kCFRunLoopAfterWaiting = (1UL << 6),    // 休眠唤醒
	    kCFRunLoopExit = (1UL << 7),            // 结束退出
	    kCFRunLoopAllActivities = 0x0FFFFFFFU
	};
	```
	
### CFRunLoopTimer
```c
struct __CFRunLoopTimer {
    uint16_t _bits;                 // Timer 的状态(firing、fired-during-callout、waking)
    CFRunLoopRef _runLoop;          // 所在的 RunLoop
    CFMutableSetRef _rlModes;       // 所在的 Mode 集合
    CFAbsoluteTime _nextFireDate;   // 下一次触发时机=当前时间+interval
    CFTimeInterval _interval;       // 理想的触发间隔
    CFTimeInterval _tolerance;      // 时间偏差
    CFRunLoopTimerCallBack _callout;	// Timer 的回调
    ...
};
```

1. 一个 Timer 可以被添加到多个 Mode
2. bits 字段表示 Timer 的状态，主要有 0:firing、1:fired-during-callout、2:waking？
3. nextFireDate 和 interval：一个 Timer 注册到 RunLoop 后，RunLoop 会为其重复的时间点注册好事件。例如 10:00, 10:10, 10:20 这几个时间点，这里 interval 就是 10 秒。nextFireDate 依次是 10:00, 10:10, 10:20
4. tolerance：RunLoop 为了节省资源，并不会在非常准确的时间点回调这个 Timer。Timer 有个属性叫做 Tolerance (宽容度)，标示了当时间点到后，容许有多少最大误差
5. 如果某个时间点被错过了，例如执行了一个很长的任务，则那个时间点的回调也会跳过去，不会延后执行

## CFRunLoopDoTimers

```c
/*
limitTSR：终止时间，如果 timer 下一次触发时间超过这个值，则 timer 不会生效
*/
static Boolean __CFRunLoopDoTimers(CFRunLoopRef rl, CFRunLoopModeRef rlm, uint64_t limitTSR)
{ 
    Boolean timerHandled = false;
    CFMutableArrayRef timers = NULL;
    // 1. 遍历 rlm 的所有 Timer，取出下一次触发时间小于当前系统时间的 Timer 组成数组
    for (CFIndex idx = 0, cnt = rlm->_timers ? CFArrayGetCount(rlm->_timers) : 0; idx < cnt; idx++)
    {
        CFRunLoopTimerRef rlt = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(rlm->_timers, idx);

        if (__CFIsValid(rlt) && !__CFRunLoopTimerIsFiring(rlt))
        {
            if (rlt->_fireTSR <= limitTSR)
            {
                if (!timers) 
                    timers = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
                CFArrayAppendValue(timers, rlt);
            }
        }
    }

    // 2. 对该数组的 Timer 进行遍历，调用 __CFRunLoopDoTimer(rl, rlm, rlt);
    for (CFIndex idx = 0, cnt = timers ? CFArrayGetCount(timers) : 0; idx < cnt; idx++)
    {
        CFRunLoopTimerRef rlt = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(timers, idx);
        Boolean did = __CFRunLoopDoTimer(rl, rlm, rlt);
        timerHandled = timerHandled || did;
    }
    if (timers) CFRelease(timers);
    return timerHandled;
}
```
1. limitTSR 这里传的是 `mach_absolute_time()`，该函数返回一个基于系统启动后的时钟嘀嗒数，表示当前系统时间


精简伪代码

```c
static Boolean __CFRunLoopDoTimers(CFRunLoopRef rl, CFRunLoopModeRef rlm, uint64_t limitTSR)
{
    1. 遍历 rlm 的所有 Timer，取出下一次触发时间小于当前系统时间的 Timer 组成数组
    2. 对该数组的 Timer 进行遍历，调用 __CFRunLoopDoTimer(rl, rlm, rlt);
}
```

## CFRunLoopDoTimer

```c
static Boolean __CFRunLoopDoTimer(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFRunLoopTimerRef rlt)
{
    Boolean timerHandled = false;
    uint64_t oldFireTSR = 0;

    if (__CFIsValid(rlt) && rlt->_fireTSR <= mach_absolute_time() && !__CFRunLoopTimerIsFiring(rlt) && rlt->_runLoop == rl)
    {
    	// interval = 0 的 Timer 都是一次性的
        Boolean doInvalidate = (0.0 == rlt->_interval);
        // 设置正在 fire 的标志位
        __CFRunLoopTimerSetFiring(rlt);
        // 记录本次的调用时间，待会用到
        oldFireTSR = rlt->_fireTSR;
        
        // 计算下一次 Timer 应该触发的时机并注册对应的 timer（mk_timer or GCD Timer）
        __CFArmNextTimerInMode(rlm, rl);

        // 调用 timer 的回调
        __CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__(rlt->_callout, rlt, context_info);

        // 如果 timer 如果 interval 为 0，则移除 timer（无论是不是 repeat，只要 interval 为 0 就只调用一次）
        if (doInvalidate)
        {
            CFRunLoopTimerInvalidate(rlt);
        }
        timerHandled = true;

        // 取消 fire 的标志位
        __CFRunLoopTimerUnsetFiring(rlt);
    }

    // 非一次性 Timer 在执行回调之后需要更新 fireTSR 和 nextFireDate
    if (__CFIsValid(rlt) && timerHandled)
    {
        if (oldFireTSR < rlt->_fireTSR)
        {
        	// 如果刚刚在 Timer 的回调期间，调整了该 Timer（调整的结果就是这个 timer 的 fireTSR 一定会变大）
        	// 这种情况下如果此时该 Timer 依然是最小的，那么刚刚的 __CFArmNextTimerInMode 的计算就是错的
        	// 因为该 Timer 被设置了 fire 而被忽略，没有纳入计算
        	// 所以这里补充一次重新计算
            __CFArmNextTimerInMode(rlm, rl);
        }
        else
        {
            uint64_t nextFireTSR = 0LL;
            uint64_t intervalTSR = 0LL;

            // 确保 intervalTSR 不会超过最大值
            if (rlt->_interval <= 0.0)
            {
            	// 如果在 Timer 回调期间修改了 interval，可能会走到这里，do nothing
            }
            else if (TIMER_INTERVAL_LIMIT < rlt->_interval)
            {
                intervalTSR = __CFTimeIntervalToTSR(TIMER_INTERVAL_LIMIT);
            }
            else
            {
                intervalTSR = __CFTimeIntervalToTSR(rlt->_interval);
            }

            // 计算下一次触发时间
            if (LLONG_MAX - intervalTSR <= oldFireTSR)
            {
            	// 如果 interval 很大很大，超过上限
                nextFireTSR = LLONG_MAX;
            }
            else
            {
                if (intervalTSR == 0)
                {
                	// 一般不可能走到，只是为了防止死循环
                    CRSetCrashLogMessage("A CFRunLoopTimer with an interval of 0 is set to repeat");
                    HALT;
                }
                uint64_t currentTSR = mach_absolute_time();
                nextFireTSR = oldFireTSR;
                // 确保下次触发的时间一定大于当前时间（每次增加 interval 的倍数）
                while (nextFireTSR <= currentTSR)
                {
                    nextFireTSR += intervalTSR;
                }
            }


            // 更新 fireTSR 和 nextFireDate
            CFRunLoopRef rlt_rl = rlt->_runLoop;
            if (rlt_rl)
            {
            	// 如果 Timer 是在 RunLoop 里，则更新了 fireTSR 和 nextFireDate 之后
            	// 需要对 RunLoop 的每个包含该 Timer 的 Mode 的 Timers 重新排序

                CFIndex cnt = CFSetGetCount(rlt->_rlModes);
                STACK_BUFFER_DECL(CFTypeRef, modes, cnt);
                CFSetGetValues(rlt->_rlModes, (const void **)modes);
                
                // 寻找包含了该 Timer 的 Mode
                for (CFIndex idx = 0; idx < cnt; idx++)
                {
                    CFStringRef name = (CFStringRef)modes[idx];
                    modes[idx] = (CFTypeRef)__CFRunLoopFindMode(rlt_rl, name, false);
                }

                // 更新自己
                rlt->_fireTSR = nextFireTSR;
                rlt->_nextFireDate = CFAbsoluteTimeGetCurrent() + __CFTimeIntervalUntilTSR(nextFireTSR);
                // 排序
                for (CFIndex idx = 0; idx < cnt; idx++)
                {
                    CFRunLoopModeRef rlm = (CFRunLoopModeRef)modes[idx];
                    if (rlm)
                    {
                    	// 对 mode 中的 timer 重新排序
                        __CFRepositionTimerInMode(rlm, rlt, true);
                    }
                }
            }
            else
            {
            		// 没有 RunLoop，更新自己即可
                rlt->_fireTSR = nextFireTSR;
                rlt->_nextFireDate = CFAbsoluteTimeGetCurrent() + __CFTimeIntervalUntilTSR(nextFireTSR);
            }
        }
    }
    return timerHandled;
}
```

1. 触发 Timer 的回调；更新此 Timer 的下一次触发时间；同时对包含该 Timer 的 Mode 的 Timers 重新排序
2. 对于重复的 Timer，其多次触发的时刻不是一开始算好的，而是 Timer 触发后计算的。但是计算时参考的是上次应当触发的时间 _fireTSR（而不是当前时间），因此计算出的下次触发的时刻不会有误差。这保证了 Timer 不会出现误差叠加。比如本来 5 秒触发一次，第一次却延迟到第 7 秒才触发，但是第二次依然在 10 秒触发，而不是 12 秒。具体可以看 `_fireTSR` 的赋值是由 nextFireTSR 决定，而 nextFireTSR 是由上一次的 `_fireTSR` 决定
3. 对于重复的 Timer，如果 RunLoop 很忙，那么 Timer 的一些回调可能被忽略。当 RunLoop 不忙了，开始处理 Timer 的时候，即上述函数，此时因为 nextFireTSR 的是一定要比当前时间晚，所以小于当前时间的触发时机都会被忽略，比如本来 5 秒触发一次，第一次在第 5 秒时正常触发，第二次却延迟到了第 16 秒才触发，那么第 15 秒的触发就会被取消，第三次应该触发的时机是 20 秒，如下图和代码

```c
while (nextFireTSR <= currentTSR)
{
	nextFireTSR += intervalTSR;
}
```

![](https://user-gold-cdn.xitu.io/2017/9/11/2569c785d146789bc9bb56be09afb31d)

## CFArmNextTimerInMode

```c
static void __CFArmNextTimerInMode(CFRunLoopModeRef rlm, CFRunLoopRef rl)
{
    uint64_t nextHardDeadline = UINT64_MAX;
    uint64_t nextSoftDeadline = UINT64_MAX;

    if (rlm->_timers)
    {
        // Timers 是按照 softDeadline 排序
        // 遍历 Timers，计算 Mode 的 softDeadLine 和 hardDeadLine
        // softDeadline 是理应触发的时间；hardDeadline 是理应触发的时间加上 tolerance
        // 即计算下一次应该触发 Timer 的精准时机和模糊时机（这两个时机不一定来自同一个 Timer）
        for (CFIndex idx = 0, cnt = CFArrayGetCount(rlm->_timers); idx < cnt; idx++)
        {
            CFRunLoopTimerRef t = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(rlm->_timers, idx);
            // 如果正在调用则过滤，详见 __CFRunLoopDoTimer
            if (__CFRunLoopTimerIsFiring(t)) continue;

            int32_t err = CHECKINT_NO_ERROR;

            // softDeadline 是理应触发的时间；hardDeadline 是理应触发的时间加上 tolerance
            uint64_t oneTimerSoftDeadline = t->_fireTSR;
            uint64_t oneTimerHardDeadline = check_uint64_add(t->_fireTSR, __CFTimeIntervalToTSR(t->_tolerance), &err);
            if (err != CHECKINT_NO_ERROR) oneTimerHardDeadline = UINT64_MAX;

            // Timers 是按照 softDeadline 排序，如果此时 softDeadline 已经很大
            // 说明下一次的触发时机一定跟这个 Timer 无关，因此可以跳过
            // oneTimerSoftDeadline > nextHardDeadline >= nextSoftDeadline
            if (oneTimerSoftDeadline > nextHardDeadline)
            {
                break;
            }

            // 找最小的那个
            if (oneTimerSoftDeadline < nextSoftDeadline)
            {
                nextSoftDeadline = oneTimerSoftDeadline;
            }

            if (oneTimerHardDeadline < nextHardDeadline)
            {
                nextHardDeadline = oneTimerHardDeadline;
            }
        }

        if (nextSoftDeadline < UINT64_MAX && (nextHardDeadline != rlm->_timerHardDeadline || nextSoftDeadline != rlm->_timerSoftDeadline))
        {
            // leeway = tolerance（这个 tolerance 可能是两个 Timer 的 deadline 相减得到）
            uint64_t leeway = __CFTSRToNanoseconds(nextHardDeadline - nextSoftDeadline);
            dispatch_time_t deadline = __CFTSRToDispatchTime(nextSoftDeadline);
            if (leeway > 0)
            {
                // 如果有 tolerance，则取消 mk_timer
                if (rlm->_mkTimerArmed && rlm->_timerPort)
                {
                    AbsoluteTime dummy;
                    mk_timer_cancel(rlm->_timerPort, &dummy);
                    rlm->_mkTimerArmed = false;
                }

                // 底层通过 dispatch_source_set_timer 注册 timer
                _dispatch_source_set_runloop_timer_4CF(rlm->_timerSource, deadline, DISPATCH_TIME_FOREVER, leeway);
                rlm->_dispatchTimerArmed = true;
            }
            else
            {
                // 如果没有 tolerance，则取消 dispatch timer
                if (rlm->_dispatchTimerArmed)
                {
                    // Cancel the dispatch timer
                    _dispatch_source_set_runloop_timer_4CF(rlm->_timerSource, DISPATCH_TIME_FOREVER, DISPATCH_TIME_FOREVER, 888);
                    rlm->_dispatchTimerArmed = false;
                }

                // 注册 mk_timer
                if (rlm->_timerPort)
                {
                    mk_timer_arm(rlm->_timerPort, __CFUInt64ToAbsoluteTime(nextSoftDeadline));
                    rlm->_mkTimerArmed = true;
                }
            }
        }
        else if (nextSoftDeadline == UINT64_MAX)
        {
            // nextSoftDeadline <= nextHardDeadline，因此 nextHardDeadline 也是 UINT64_MAX
            // 走到这里说明没有合法的 Timer 可以被下次 RunLoop 调用
            // 取消所有类型的 Timer
            if (rlm->_mkTimerArmed && rlm->_timerPort)
            {
                AbsoluteTime dummy;
                mk_timer_cancel(rlm->_timerPort, &dummy);
                rlm->_mkTimerArmed = false;
            }

            if (rlm->_dispatchTimerArmed)
            {
                _dispatch_source_set_runloop_timer_4CF(rlm->_timerSource, DISPATCH_TIME_FOREVER, DISPATCH_TIME_FOREVER, 333);
                rlm->_dispatchTimerArmed = false;
            }
        }
    }
    rlm->_timerHardDeadline = nextHardDeadline;
    rlm->_timerSoftDeadline = nextSoftDeadline;
}
```

1. 首先要知道 Mode 的 Timers 是按照 softDeadline 排序；
2. 该函数通过遍历 Mode 下的所有 Timer，计算出下一次应该触发 Timer 的 softDeadline 和 hardDeadline（这两个时机不一定来自同一个 Timer）；
3. 如果有 tolerance ，则注册一个 GCD Timer；否则注册一个 mk_timer



## RunLoop 的入口

在 Core Foundation 中我们可以通过以下2个 API 来让 RunLoop 运行

1. 在默认的 mode 下运行当前线程的 RunLoop

	```c
	void CFRunLoopRun(void) {	
	    int32_t result;
	    do {
	        result = CFRunLoopRunSpecific(CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 1.0e10, false);
	    } while (kCFRunLoopRunStopped != result && kCFRunLoopRunFinished != result);
	}
	```

2. 在指定 mode 下运行当前线程的 RunLoop

	```c
	SInt32 CFRunLoopRunInMode(CFStringRef modeName, CFTimeInterval seconds, Boolean returnAfterSourceHandled) {     
	    return CFRunLoopRunSpecific(CFRunLoopGetCurrent(), modeName, seconds, returnAfterSourceHandled);
	}
	```

虽然 RunLoop 有很多个 mode，但是 RunLoop 在 run 的时候必须只能指定其中一个 mode，运行起来之后，被指定的 mode 即为 currentMode

## CFRunLoopRunSpecific
```c
SInt32 CFRunLoopRunSpecific(CFRunLoopRef rl, CFStringRef modeName, CFTimeInterval seconds, Boolean returnAfterSourceHandled)
{ 
    // 是否已经析构
    if (__CFRunLoopIsDeallocating(rl)) return kCFRunLoopRunFinished;
    
    CFRunLoopModeRef currentMode = __CFRunLoopFindMode(rl, modeName, false);

    // 判断指定的 Mode 存不存在，或者 ModeItem 是不是空的
    if (NULL == currentMode || __CFRunLoopModeIsEmpty(rl, currentMode, rl->_currentMode))
    {
        return kCFRunLoopRunFinished;
    }
    
    // 备份上一个 Mode 的数据
    volatile _per_run_data *previousPerRun = __CFRunLoopPushPerRunData(rl);
    CFRunLoopModeRef previousMode = rl->_currentMode;

    rl->_currentMode = currentMode;
    int32_t result = kCFRunLoopRunFinished;

    // 如果注册了对应的 Observer，则通知即将进入 RunLoop
    if (currentMode->_observerMask & kCFRunLoopEntry) __CFRunLoopDoObservers(rl, currentMode, kCFRunLoopEntry);
    // 真正处理 RunLoop
    result = __CFRunLoopRun(rl, currentMode, seconds, returnAfterSourceHandled, previousMode);
    // 如果注册了对应的 Observer，则通知即将结束 RunLoop
    if (currentMode->_observerMask & kCFRunLoopExit) __CFRunLoopDoObservers(rl, currentMode, kCFRunLoopExit);

    // 恢复上一个 Mode 的数据
    __CFRunLoopPopPerRunData(rl, previousPerRun);
    rl->_currentMode = previousMode;

    return result;
}
```

1. 如果一个 RunLoop Mode 是空的，则 `__CFRunLoopModeIsEmpty` 这一步就会返回 true 而导致 RunLoop 退出
2. 通知 Observer 即将进入和即将退出的代码可以在这里找到

## CFRunLoopModeIsEmpty
```c
static Boolean __CFRunLoopModeIsEmpty(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFRunLoopModeRef previousMode)
{
    if (NULL == rlm) return true;

    // 如果是主线程，直接 return false
    Boolean libdispatchQSafe = pthread_main_np() && ((HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && NULL == previousMode) || (!HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && 0 == _CFGetTSD(__CFTSDKeyIsInGCDMainQ)));
    if (libdispatchQSafe && (CFRunLoopGetMain() == rl) && CFSetContainsValue(rl->_commonModes, rlm->_name)) return false;

    // 如果 source0/source1/timer 有一个，则 return false
    if (NULL != rlm->_sources0 && 0 < CFSetGetCount(rlm->_sources0)) return false;
    if (NULL != rlm->_sources1 && 0 < CFSetGetCount(rlm->_sources1)) return false;
    if (NULL != rlm->_timers && 0 < CFArrayGetCount(rlm->_timers)) return false;

    // 传入的 mode 是否存在于当前的 RunLoop 的 Block Mode 中，或者存在于当前 RunLoop 的 CommonModes 中
    struct _block_item *item = rl->_blocks_head;
    while (item)
    {
        struct _block_item *curr = item;
        item = item->_next;
        Boolean doit = false;

        // 判断 Mode 是不是一个 string 类型
        // 比较传入的 mode 是否存在于当前的 RunLoop，或者存在于当前 RunLoop 的 CommonModes 中
        if (CFStringGetTypeID() == CFGetTypeID(curr->_mode))
        {

            doit = CFEqual(curr->_mode, rlm->_name) || (CFEqual(curr->_mode, kCFRunLoopCommonModes) && CFSetContainsValue(rl->_commonModes, rlm->_name));
        }
        else
        {
            doit = CFSetContainsValue((CFSetRef)curr->_mode, rlm->_name) || (CFSetContainsValue((CFSetRef)curr->_mode, kCFRunLoopCommonModes) && CFSetContainsValue(rl->_commonModes, rlm->_name));
        }
        if (doit) return false;
    }

    return true;
}
```
1. 几乎网上所有的文章都说，如果没有 Source0/Source1/Timer，则 RunLoop Mode 是空的，但是其实不尽然，这里还有一个 Block 类型的判断在别的文章里没有被提到，从 macOS 10.6/iOS 4 开始，可以使用 CFRunLoopPerformBlock 函数往 run loop 中添加 blocks。正确的说法应该是，如果没有 Source0/Source1/Timer 以及该 RunLoop 的 `_blocks_head` 链表中也找不到该 Mode，才能判断该 Mode 是空的
2. 注意我们说的不是 RunLoop 是空的，准确的说应该是 RunLoop Mode 是空的，由于一个 RunLoop 一次只能运行一个 Mode，所以这两种说法在某种意义上是等价的
    
## CFRunLoopRun
```c
/**
 RunLoop 的主流程，无限运行的秘密就在此

 @param rl RunLoop
 @param rlm RunLoop Mode
 @param seconds RunLoop 超时时间
 @param stopAfterHandle 处理后是否结束
 @param previousMode 上一个 RunLoop Mode
 @return
 */
static int32_t __CFRunLoopRun(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFTimeInterval seconds, Boolean stopAfterHandle, CFRunLoopModeRef previousMode)
{
    // 记录当前时间
    uint64_t startTSR = mach_absolute_time();

    // 处理一些异常情况
    if (__CFRunLoopIsStopped(rl))
    {
        __CFRunLoopUnsetStopped(rl);
        return kCFRunLoopRunStopped;
    }
    else if (rlm->_stopped)
    {
        rlm->_stopped = false;
        return kCFRunLoopRunStopped;
    }

    // 声明用于和 mach_port 通信的端口
    mach_port_name_t dispatchPort = MACH_PORT_NULL;
    // 如果是主线程，给端口赋值
    Boolean libdispatchQSafe = pthread_main_np() && ((HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && NULL == previousMode) || (!HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && 0 == _CFGetTSD(__CFTSDKeyIsInGCDMainQ)));
    // dispatch_get_main_queue_handle_4CF 返回的是主线程 RunLoop 所关联的的端口
    if (libdispatchQSafe && (CFRunLoopGetMain() == rl) && CFSetContainsValue(rl->_commonModes, rlm->_name)) dispatchPort = _dispatch_get_main_queue_port_4CF();

    // 处理 Timer 相关
    dispatch_source_t timeout_timer = NULL;
    struct __timeout_context *timeout_context = (struct __timeout_context *)malloc(sizeof(*timeout_context));
    if (seconds <= 0.0)
    {
        // 不超时
        seconds = 0.0;
        timeout_context->termTSR = 0ULL;
    }
    else if (seconds <= TIMER_INTERVAL_LIMIT)
    {
        // 正常限制内的超时
        // 根据是否是主线程来取主队列 or 后台队列
        dispatch_queue_t queue = pthread_main_np() ? __CFDispatchQueueGetGenericMatchingMain() : __CFDispatchQueueGetGenericBackground();
        // 创建 GCD Timer
        timeout_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
        // Retain，防止释放
        dispatch_retain(timeout_timer);
        // 记录在超时上下文
        timeout_context->ds = timeout_timer;
        timeout_context->rl = (CFRunLoopRef)CFRetain(rl);
        timeout_context->termTSR = startTSR + __CFTimeIntervalToTSR(seconds);
        // 超时上下文记录在 Timer
        dispatch_set_context(timeout_timer, timeout_context); // source gets ownership of context
        // 设置超时回调
        dispatch_source_set_event_handler_f(timeout_timer, __CFRunLoopTimeout);
        // 设置超时取消回调
        dispatch_source_set_cancel_handler_f(timeout_timer, __CFRunLoopTimeoutCancel);
        uint64_t ns_at = (uint64_t)((__CFTSRToTimeInterval(startTSR) + seconds) * 1000000000ULL);
        dispatch_source_set_timer(timeout_timer, dispatch_time(1, ns_at), DISPATCH_TIME_FOREVER, 1000ULL);
        // 启动定时器
        dispatch_resume(timeout_timer);
    }
    else
    { 
        // 无限超时
        seconds = 9999999999.0;
        timeout_context->termTSR = UINT64_MAX;
    }

    Boolean didDispatchPortLastTime = true;
    int32_t retVal = 0;
    // 让我们开启无限循环的秘密
    do
    {
        voucher_mach_msg_state_t voucherState = VOUCHER_MACH_MSG_STATE_UNCHANGED;
        voucher_t voucherCopy = NULL;
        uint8_t msg_buffer[3 * 1024];
        mach_msg_header_t *msg = NULL;
        mach_port_t livePort = MACH_PORT_NULL;

        __CFPortSet waitSet = rlm->_portSet;

        // 取消 RunLoop 的忽略唤醒信号，从此在线接收唤醒（可以接收 port 消息）
        __CFRunLoopUnsetIgnoreWakeUps(rl);

        // 通知即将处理 Timer
        if (rlm->_observerMask & kCFRunLoopBeforeTimers) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeTimers);
        // 通知即将处理 Source
        if (rlm->_observerMask & kCFRunLoopBeforeSources) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeSources);

        // 处理 Blocks
        __CFRunLoopDoBlocks(rl, rlm);

        // 处理 Source0（非 port）
        Boolean sourceHandledThisLoop = __CFRunLoopDoSources0(rl, rlm, stopAfterHandle);
        if (sourceHandledThisLoop)
        {
            // 如果处理了 Source0 之后还有 Block 要处理，则再次处理 Block
            __CFRunLoopDoBlocks(rl, rlm);
        }

        // poll = 是否处理 Source0 或没有超时
        Boolean poll = sourceHandledThisLoop || (0ULL == timeout_context->termTSR);

        // 如果是主线程的队列里有未处理的消息且上一次循环的睡眠不是 dispatch 唤醒的
        if (MACH_PORT_NULL != dispatchPort && !didDispatchPortLastTime) {
            msg = (mach_msg_header_t *)msg_buffer;
            // timeout 为 0，不会休眠，直接处理消息
            if (__CFRunLoopServiceMachPort(dispatchPort, &msg, sizeof(msg_buffer), &livePort, 0, &voucherState, NULL)) {
                goto handle_msg;
            }
        }
        didDispatchPortLastTime = false;

        // 通知即将休眠
        if (!poll && (rlm->_observerMask & kCFRunLoopBeforeWaiting)) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeWaiting);
        // 设置睡眠标志
        __CFRunLoopSetSleeping(rl);

        // 每次循环都将 dispatchPort 加入监听端口集合中
        __CFPortSetInsert(dispatchPort, waitSet);

        // 开始睡眠时间
        CFAbsoluteTime sleepStart = poll ? 0.0 : CFAbsoluteTimeGetCurrent();

        if (kCFUseCollectableAllocator) {
            memset(msg_buffer, 0, sizeof(msg_buffer));
        }
        msg = (mach_msg_header_t *)msg_buffer;

        // 调用 mach_msg 等待接受 mach_port 的消息。线程将进入休眠, 直到被下面某一个事件唤醒。
        // • 一个基于 port 的 Source 的事件。
        // • 一个 Timer 到时间了
        // • RunLoop 自身的超时时间到了
        // • 被其他什么调用者手动唤醒
        __CFRunLoopServiceMachPort(waitSet, &msg, sizeof(msg_buffer), &livePort, poll ? 0 : TIMEOUT_INFINITY, &voucherState, &voucherCopy);

        // 睡了多久
        rl->_sleepTime += (poll ? 0.0 : (CFAbsoluteTimeGetCurrent() - sleepStart));

        // 每次循环都移除刚刚的 dispatchPort
        __CFPortSetRemove(dispatchPort, waitSet);

        // 设置 RunLoop 的忽略唤醒信号，从此下线不接收唤醒（不再接收 port 消息）
        __CFRunLoopSetIgnoreWakeUps(rl);

        // 取消睡眠标志
        __CFRunLoopUnsetSleeping(rl);

        // 通知结束休眠
        if (!poll && (rlm->_observerMask & kCFRunLoopAfterWaiting)) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopAfterWaiting);

        // 处理消息
    handle_msg:;
        // 设置 RunLoop 的忽略唤醒信号，从此下线不接收唤醒（不再接收 port 消息）
        __CFRunLoopSetIgnoreWakeUps(rl);

        if (MACH_PORT_NULL == livePort)
        {
            CFRUNLOOP_WAKEUP_FOR_NOTHING();
            // handle nothing
        }
        else if (livePort == rl->_wakeUpPort)
        {
            CFRUNLOOP_WAKEUP_FOR_WAKEUP();
            // do nothing on Mac OS

        }
        else if (rlm->_timerPort != MACH_PORT_NULL && livePort == rlm->_timerPort)
        {
            CFRUNLOOP_WAKEUP_FOR_TIMER();
            if (!__CFRunLoopDoTimers(rl, rlm, mach_absolute_time()))
            {
                // Re-arm the next timer
                __CFArmNextTimerInMode(rlm, rl);
            }
        }
        else if (livePort == dispatchPort)
        {
            CFRUNLOOP_WAKEUP_FOR_DISPATCH();
            _CFSetTSD(__CFTSDKeyIsInGCDMainQ, (void *)6, NULL);
            __CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE__(msg);
            _CFSetTSD(__CFTSDKeyIsInGCDMainQ, (void *)0, NULL);
            sourceHandledThisLoop = true;
            didDispatchPortLastTime = true;
        }
        else
        {
            CFRUNLOOP_WAKEUP_FOR_SOURCE();
            voucher_t previousVoucher = _CFSetTSD(__CFTSDKeyMachMessageHasVoucher, (void *)voucherCopy, os_release);
            CFRunLoopSourceRef rls = __CFRunLoopModeFindSourceForMachPort(rl, rlm, livePort);

            if (rls) {
                mach_msg_header_t *reply = NULL;
                sourceHandledThisLoop = __CFRunLoopDoSource1(rl, rlm, rls, msg, msg->msgh_size, &reply) || sourceHandledThisLoop;
                if (NULL != reply) {
                    (void)mach_msg(reply, MACH_SEND_MSG, reply->msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
                    CFAllocatorDeallocate(kCFAllocatorSystemDefault, reply);
                }
            }
            // Restore the previous voucher
            _CFSetTSD(__CFTSDKeyMachMessageHasVoucher, previousVoucher, os_release);
        }

        // 处理 Blocks
        __CFRunLoopDoBlocks(rl, rlm);


        if (sourceHandledThisLoop && stopAfterHandle)
        {
            // 进入该函数时参数说处理完事件就返回
            retVal = kCFRunLoopRunHandledSource;
        }
        else if (timeout_context->termTSR < mach_absolute_time())
        {
            // 超出传入参数标记的超时时间了
            retVal = kCFRunLoopRunTimedOut;
        }
        else if (__CFRunLoopIsStopped(rl))
        {
            // 被外部调用者强制停止了
            __CFRunLoopUnsetStopped(rl);
            retVal = kCFRunLoopRunStopped;
        }
        else if (rlm->_stopped)
        {
            // Mode 已经被标记为 Stop
            rlm->_stopped = false;
            retVal = kCFRunLoopRunStopped;
        }
        else if (__CFRunLoopModeIsEmpty(rl, rlm, previousMode))
        {
            // source/timer/observer一个都没有了
            retVal = kCFRunLoopRunFinished;
        }

        voucher_mach_msg_revert(voucherState);
        os_release(voucherCopy);
    } while (0 == retVal);

    // 释放 timer
    if (timeout_timer)
    {
        // 如果有 timer，timeout_context 是被 timer 内部管理，不需要手动释放
        dispatch_source_cancel(timeout_timer);
        dispatch_release(timeout_timer);
    }
    else
    {
        // 手动管理 timeout_context
        free(timeout_context);
    }

    return retVal;
}
```

1. `mach_msg`

	```c
	mach_msg_return_t mach_msg(mach_msg_header_t msg,
	                           mach_msg_option_t option,
	                           mach_msg_size_t send_size,
	                           mach_msg_size_t receive_limit,
	                           mach_port_t receive_name,
	                           mach_msg_timeout_t timeout,
	                           mach_port_t notify);
	```
	
	详细可参考 [源码](http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/mach_msg.html)
	
	`mach_msg` 是系统内核在某个 port 收发消息所使用的函数，收消息与发消息都是调用这个函数，只是 `mach_msg_option_t` 参数不同，发送为 `MACH_SEND_MSG`，接收为 `MACH_RCV_MSG`

	可以简单的将 `mach_msg` 理解为多进程之间的一种通信机制，不同的进程可以使用同一个消息队列来交流数据，当使用 `mach_msg` 从消息队列里读取 msg 时，可以在参数中 timeout 值，在 timeout 之前如果没有读到 msg，当前线程会一直处于休眠状态。这也是 runloop 在没有任务可执行的时候，能够进入 sleep 状态的原因。如果 timeout = 0，则不会进入休眠；如果 timeout = TIMEOUT_INFINITY，则在没消息之前一直休眠
		
2. 无限循环的真相

	代码中一共出现了两处 `__CFRunLoopServiceMachPort`，注意 timeout 的值
	
	```c
	// TimeOut 为 0
	__CFRunLoopServiceMachPort(dispatchPort, &msg, sizeof(msg_buffer), &livePort, 0, &voucherState, NULL)
    
	// TimeOut 为 0 or 无限大
	__CFRunLoopServiceMachPort(waitSet, &msg, sizeof(msg_buffer), &livePort, poll ? 0 : TIMEOUT_INFINITY, &voucherState, &voucherCopy);
	```
	

	`__CFRunLoopServiceMachPort` 的实质是调用了 `mach_msg`

	```c
	ret = mach_msg(msg, 
	MACH_RCV_MSG|(voucherState ? MACH_RCV_VOUCHER : 0)|MACH_RCV_LARGE|((TIMEOUT_INFINITY != timeout) ? MACH_RCV_TIMEOUT : 0)|MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0)|MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AV), 
	0, 
	msg->msgh_size, 
	port, 
	timeout, 
	MACH_PORT_NULL);
	```
	
	第一处 `mach_msg` ，即如果主队列有任务执行，则不会进入睡眠，应该是为了保障 dispatch 到 main queue 的代码总是有较高的机会得以运行
	
3. 可以看到 source0 是没有唤醒 RunLoop 的能力的，而 source1 有
4. 可以看到并不是每次 RunLoop 如果处理了 source0 任务，那么 poll 值会为 true，会直接进入睡眠，而且不会告知 BeforeWaiting 和 AfterWaiting。所以有些情况下经过了几次循环，但注册的 observer 却不会收到回调