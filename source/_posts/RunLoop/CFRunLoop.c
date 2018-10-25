/* rl, rlm are locked on entrance and exit */
static int32_t __CFRunLoopRun(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFTimeInterval seconds, Boolean stopAfterHandle, CFRunLoopModeRef previousMode)
{
    //// 记录开始时间
    uint64_t startTSR = mach_absolute_time();

    //// 检查是否被停止
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

    mach_port_name_t dispatchPort = MACH_PORT_NULL;
    Boolean libdispatchQSafe = pthread_main_np() && ((HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && NULL == previousMode) || (!HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && 0 == _CFGetTSD(__CFTSDKeyIsInGCDMainQ)));
    if (libdispatchQSafe && (CFRunLoopGetMain() == rl) && CFSetContainsValue(rl->_commonModes, rlm->_name)) dispatchPort = _dispatch_get_main_queue_port_4CF();

    //// 如果使用 GCD timer 作为 timer 的实现的话，进行准备工作
#if USE_DISPATCH_SOURCE_FOR_TIMERS
    mach_port_name_t modeQueuePort = MACH_PORT_NULL;
    if (rlm->_queue)
    {
        modeQueuePort = _dispatch_runloop_root_queue_get_port_4CF(rlm->_queue);
        if (!modeQueuePort)
        {
            CRASH("Unable to get port for run loop mode queue (%d)", -1);
        }
    }
#endif

    //// 设置超时开始
    dispatch_source_t timeout_timer = NULL;
    struct __timeout_context *timeout_context = (struct __timeout_context *)malloc(sizeof(*timeout_context));
    if (seconds <= 0.0)
    {
        // instant timeout
        seconds = 0.0;
        timeout_context->termTSR = 0ULL;
    }
    else if (seconds <= TIMER_INTERVAL_LIMIT)
    {
        dispatch_queue_t queue = pthread_main_np() ? __CFDispatchQueueGetGenericMatchingMain() : __CFDispatchQueueGetGenericBackground();
        timeout_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
        dispatch_retain(timeout_timer);
        timeout_context->ds = timeout_timer;
        timeout_context->rl = (CFRunLoopRef)CFRetain(rl);
        timeout_context->termTSR = startTSR + __CFTimeIntervalToTSR(seconds);
        dispatch_set_context(timeout_timer, timeout_context); // source gets ownership of context
        dispatch_source_set_event_handler_f(timeout_timer, __CFRunLoopTimeout);
        dispatch_source_set_cancel_handler_f(timeout_timer, __CFRunLoopTimeoutCancel);
        uint64_t ns_at = (uint64_t)((__CFTSRToTimeInterval(startTSR) + seconds) * 1000000000ULL);
        dispatch_source_set_timer(timeout_timer, dispatch_time(1, ns_at), DISPATCH_TIME_FOREVER, 1000ULL);
        dispatch_resume(timeout_timer);
    }
    else
    {
        // infinite timeout
        seconds = 9999999999.0;
        timeout_context->termTSR = UINT64_MAX;
    }
    //// 设置超时结束

    Boolean didDispatchPortLastTime = true;
    int32_t retVal = 0;
    do
    {
        voucher_mach_msg_state_t voucherState = VOUCHER_MACH_MSG_STATE_UNCHANGED;
        voucher_t voucherCopy = NULL;
        uint8_t msg_buffer[3 * 1024];
        mach_msg_header_t *msg = NULL;
        mach_port_t livePort = MACH_PORT_NULL;
        __CFPortSet waitSet = rlm->_portSet;

        __CFRunLoopUnsetIgnoreWakeUps(rl);

        //// 2. 通知 observers: kCFRunLoopBeforeTimers, 即将处理 timers
        if (rlm->_observerMask & kCFRunLoopBeforeTimers) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeTimers);
        //// 3. 通知 observers: kCFRunLoopBeforeSources, 即将处理 sources
        if (rlm->_observerMask & kCFRunLoopBeforeSources) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeSources);

        //// 4. 处理 blocks
        __CFRunLoopDoBlocks(rl, rlm);

        //// 5. 处理 sources 0
        Boolean sourceHandledThisLoop = __CFRunLoopDoSources0(rl, rlm, stopAfterHandle);
        //// 6. 如果第 5 步实际处理了 sources 0，再一次处理 blocks
        if (sourceHandledThisLoop)
        {
            __CFRunLoopDoBlocks(rl, rlm);
        }

        //// 是否处理过 source0 或超时
        Boolean poll = sourceHandledThisLoop || (0ULL == timeout_context->termTSR);

        //// 7. 如果在主线程，检查是否有 GCD 事件需要处理，有的话，跳转到第 11 步
        if (MACH_PORT_NULL != dispatchPort && !didDispatchPortLastTime)
        {
            msg = (mach_msg_header_t *)msg_buffer;
            //// __CFRunLoopServiceMachPort 这个函数会睡眠线程，如果超时时间不为 0 的话
            if (__CFRunLoopServiceMachPort(dispatchPort,
                                           &msg,
                                           sizeof(msg_buffer),
                                           &livePort,
                                           0,
                                           &voucherState,
                                           NULL))
            {
                goto handle_msg;
            }
        }

        didDispatchPortLastTime = false;

        //// 8. 通知 observers: kCFRunLoopBeforeWaiting, 即将进入等待（睡眠）
        //// 注意到如果实际处理了 source0 或者超时了，不会进入睡眠，所以不会通知
        if (!poll && (rlm->_observerMask & kCFRunLoopBeforeWaiting)) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeWaiting);
        __CFRunLoopSetSleeping(rl);
        // do not do any user callouts after this point (after notifying of sleeping)

        // Must push the local-to-this-activation ports in on every loop
        // iteration, as this mode could be run re-entrantly and we don't
        // want these ports to get serviced.

        __CFPortSetInsert(dispatchPort, waitSet);

        __CFRunLoopModeUnlock(rlm);
        __CFRunLoopUnlock(rl);

        //// 记录睡眠开始时间
        CFAbsoluteTime sleepStart = poll ? 0.0 : CFAbsoluteTimeGetCurrent();

        //// 9. 等待被唤醒，可以被 sources 1、timers、CFRunLoopWakeUp 函数和 GCD 事件（如果在主线程）
#if USE_DISPATCH_SOURCE_FOR_TIMERS
        //// 使用 GCD timer 作为 timer 实现的情况
        do
        {
            if (kCFUseCollectableAllocator)
            {
                // objc_clear_stack(0);
                // <rdar://problem/16393959>
                memset(msg_buffer, 0, sizeof(msg_buffer));
            }
            msg = (mach_msg_header_t *)msg_buffer;

            //// 这个函数会睡眠线程
            __CFRunLoopServiceMachPort(waitSet,
                                       &msg,
                                       sizeof(msg_buffer),
                                       &livePort, poll ? 0 : TIMEOUT_INFINITY,  // 根据状态睡眠或者不睡
                                       &voucherState,
                                       &voucherCopy);

            //// 如果是 timer 端口唤醒的，进行一下善后处理，之后再处理 timer
            if (modeQueuePort != MACH_PORT_NULL && livePort == modeQueuePort)
            {
                // Drain the internal queue. If one of the callout blocks sets the timerFired flag, break out and service the timer.
                while (_dispatch_runloop_root_queue_perform_4CF(rlm->_queue))
                    ;
                if (rlm->_timerFired)
                {
                    // Leave livePort as the queue port, and service timers below
                    rlm->_timerFired = false;
                    break;
                }
                else
                {
                    if (msg && msg != (mach_msg_header_t *)msg_buffer) free(msg);
                }
            }
            else
            {
                //// 不是 timer 端口唤醒的，进行接下来的处理
                // Go ahead and leave the inner loop.
                break;
            }
        } while (1);
#else
        ///// 不使用 GCD timer 作为 timer 实现的情况
        if (kCFUseCollectableAllocator)
        {
            // objc_clear_stack(0);
            // <rdar://problem/16393959>
            memset(msg_buffer, 0, sizeof(msg_buffer));
        }
        msg = (mach_msg_header_t *)msg_buffer;
        __CFRunLoopServiceMachPort(waitSet, &msg, sizeof(msg_buffer), &livePort, poll ? 0 : TIMEOUT_INFINITY, &voucherState, &voucherCopy);
#endif

        __CFRunLoopLock(rl);
        __CFRunLoopModeLock(rlm);

        //// 增加记录的睡眠时间
        rl->_sleepTime += (poll ? 0.0 : (CFAbsoluteTimeGetCurrent() - sleepStart));

        // Must remove the local-to-this-activation ports in on every loop
        // iteration, as this mode could be run re-entrantly and we don't
        // want these ports to get serviced. Also, we don't want them left
        // in there if this function returns.

        __CFPortSetRemove(dispatchPort, waitSet);

        __CFRunLoopSetIgnoreWakeUps(rl);

        //// 10. 通知 observers: kCFRunLoopAfterWaiting, 即停止等待（被唤醒）
        //// 注意实际处理过 source 0 或者已经超时的话，不会通知（因为没有睡）
        // user callouts now OK again
        __CFRunLoopUnsetSleeping(rl);
        if (!poll && (rlm->_observerMask & kCFRunLoopAfterWaiting)) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopAfterWaiting);

        //// 11. 被什么唤醒就处理什么：
    handle_msg:;
        __CFRunLoopSetIgnoreWakeUps(rl);

        if (MACH_PORT_NULL == livePort)
        {
            //// 不知道哪个端口唤醒的（或者根本没睡），啥也不干
            CFRUNLOOP_WAKEUP_FOR_NOTHING();
            // handle nothing
        }
        else if (livePort == rl->_wakeUpPort)
        {
            //// 被 CFRunLoopWakeUp 函数弄醒的，啥也不干
            CFRUNLOOP_WAKEUP_FOR_WAKEUP();
            // do nothing on Mac OS
        }

        else if (modeQueuePort != MACH_PORT_NULL && livePort == modeQueuePort)
        {
            //// 被 timers 唤醒，处理 timers
            CFRUNLOOP_WAKEUP_FOR_TIMER();
            if (!__CFRunLoopDoTimers(rl, rlm, mach_absolute_time()))
            {
                // Re-arm the next timer, because we apparently fired early
                __CFArmNextTimerInMode(rlm, rl);
            }
        }
#if USE_MK_TIMER_TOO
        else if (rlm->_timerPort != MACH_PORT_NULL && livePort == rlm->_timerPort)
        {
            //// 被 timers 唤醒，处理 timers
            CFRUNLOOP_WAKEUP_FOR_TIMER();
            // On Windows, we have observed an issue where the timer port is set before the time which we requested it to be set. For example, we set the fire time to be TSR 167646765860, but it is actually observed firing at TSR 167646764145, which is 1715 ticks early. The result is that, when __CFRunLoopDoTimers checks to see if any of the run loop timers should be firing, it appears to be 'too early' for the next timer, and no timers are handled.
            // In this case, the timer port has been automatically reset (since it was returned from MsgWaitForMultipleObjectsEx), and if we do not re-arm it, then no timers will ever be serviced again unless something adjusts the timer list (e.g. adding or removing timers). The fix for the issue is to reset the timer here if CFRunLoopDoTimers did not handle a timer itself. 9308754
            if (!__CFRunLoopDoTimers(rl, rlm, mach_absolute_time()))
            {
                // Re-arm the next timer
                __CFArmNextTimerInMode(rlm, rl);
            }
        }
#endif
        else if (livePort == dispatchPort)
        {
            //// 被 GCD 唤醒或者从第 7 步跳转过来的话，处理 GCD
            CFRUNLOOP_WAKEUP_FOR_DISPATCH();
            __CFRunLoopModeUnlock(rlm);
            __CFRunLoopUnlock(rl);
            _CFSetTSD(__CFTSDKeyIsInGCDMainQ, (void *)6, NULL);
            __CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE__(msg);
            _CFSetTSD(__CFTSDKeyIsInGCDMainQ, (void *)0, NULL);
            __CFRunLoopLock(rl);
            __CFRunLoopModeLock(rlm);
            sourceHandledThisLoop = true;
            didDispatchPortLastTime = true;
        }
        else
        {
            //// 被 sources 1 唤醒，处理 sources 1
            CFRUNLOOP_WAKEUP_FOR_SOURCE();

            // If we received a voucher from this mach_msg, then put a copy of the new voucher into TSD. CFMachPortBoost will look in the TSD for the voucher. By using the value in the TSD we tie the CFMachPortBoost to this received mach_msg explicitly without a chance for anything in between the two pieces of code to set the voucher again.
            voucher_t previousVoucher = _CFSetTSD(__CFTSDKeyMachMessageHasVoucher, (void *)voucherCopy, os_release);

            // Despite the name, this works for windows handles as well
            CFRunLoopSourceRef rls = __CFRunLoopModeFindSourceForMachPort(rl, rlm, livePort);
            if (rls)
            {
                mach_msg_header_t *reply = NULL;
                sourceHandledThisLoop = __CFRunLoopDoSource1(rl, rlm, rls, msg, msg->msgh_size, &reply) || sourceHandledThisLoop;
                if (NULL != reply)
                {
                    (void)mach_msg(reply, MACH_SEND_MSG, reply->msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
                    CFAllocatorDeallocate(kCFAllocatorSystemDefault, reply);
                }
            }

            // Restore the previous voucher
            _CFSetTSD(__CFTSDKeyMachMessageHasVoucher, previousVoucher, os_release);
        }
        if (msg && msg != (mach_msg_header_t *)msg_buffer) free(msg);

        //// 12. 再一次处理 blocks
        __CFRunLoopDoBlocks(rl, rlm);

        //// 13. 判断是否退出，不需要退出则跳转回第 2 步
        if (sourceHandledThisLoop && stopAfterHandle)
        {
            retVal = kCFRunLoopRunHandledSource;
        }
        else if (timeout_context->termTSR < mach_absolute_time())
        {
            retVal = kCFRunLoopRunTimedOut;
        }
        else if (__CFRunLoopIsStopped(rl))
        {
            __CFRunLoopUnsetStopped(rl);
            retVal = kCFRunLoopRunStopped;
        }
        else if (rlm->_stopped)
        {
            rlm->_stopped = false;
            retVal = kCFRunLoopRunStopped;
        }
        else if (__CFRunLoopModeIsEmpty(rl, rlm, previousMode))
        {
            retVal = kCFRunLoopRunFinished;
        }

        voucher_mach_msg_revert(voucherState);
        os_release(voucherCopy);

    } while (0 == retVal);

    if (timeout_timer)
    {
        dispatch_source_cancel(timeout_timer);
        dispatch_release(timeout_timer);
    }
    else
    {
        free(timeout_context);
    }

    return retVal;
}
