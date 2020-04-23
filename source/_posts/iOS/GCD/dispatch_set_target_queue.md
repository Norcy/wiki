## 基础用法
`dispatch_set_target_queue` 的作用：

1. 改变队列优先级
2. 让多个串行队列之间也能串行地执行任务

### 改变队列优先级

`dispatch_queue_create` 创建的队列，无论是串行还是并发，其优先级都是 `DISPATCH_QUEUE_PRIORITY_DEFAULT`，使用 `dispatch_set_target_queue` 可以改变队列优先级

```objc
dispatch_queue_t serialQueue = dispatch_queue_create("", NULL);
dispatch_queue_t globalQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0);

//变更后 
dispatch_async(serialQueue, ^{  
    NSLog(@"1");  
});  
dispatch_async(dispatch_get_global_queue(0, 0), ^{  
    NSLog(@"2");  
});  

dispatch_set_target_queue(serialQueue, globalQueue);

// 变更后
dispatch_async(serialQueue, ^{  
    NSLog(@"3");  
});  
dispatch_async(dispatch_get_global_queue(0, 0), ^{  
    NSLog(@"4");  
});  

// 输出如下
// 1
// 2
// 4
// 3
```

> 第一个参数如果是主队列或者是全局并发队列则后果未知

### 让多个串行队列之间也能串行地执行任务

如果创建多个串行队列，它们之间其实是并行处理的。如果都对它们分别使用 `dispatch_set_target_queue`，指定为某一个串行队列，那么它们的任务将在目标串行队列上串行处理

```objc
dispatch_queue_t mySerialDispatchQueue1 = dispatch_queue_create("com.example.gcd.MySerialDispatchQueue1", NULL);
dispatch_queue_t mySerialDispatchQueue2 = dispatch_queue_create("com.example.gcd.MySerialDispatchQueue2", NULL);
dispatch_queue_t targetDispatchQueue = dispatch_queue_create("com.example.gcd.TargetDispatchQueue", NULL);

dispatch_set_target_queue(mySerialDispatchQueue1, targetDispatchQueue);
dispatch_set_target_queue(mySerialDispatchQueue2, targetDispatchQueue);

dispatch_async(mySerialDispatchQueue1, ^{
    NSLog(@"1 %@", [NSThread currentThread]);
});
dispatch_async(mySerialDispatchQueue2, ^{
    NSLog(@"2 %@", [NSThread currentThread]);
});

// Queue1 和 Queue2 之间有依赖关系，它们的人物会在目标串行队列上串行处理
// 输出
// 1 targetDispatchQueue 所在的线程
// 2 targetDispatchQueue 所在的线程
```

## 进阶探究
### 不同串行队列的任务顺序
当我们创建队列时，队列会附加到某一个全局队列。默认情况下会附加到默认优先级队列上

当往我们创建的队列添加任务时，这个任务会放到该队列的队尾

当该任务到达队列的头时，会把该任务移动到目标队列进行执行；并继续处理该队列的下一个任务

```objc
dispatch_queue_t mySerialDispatchQueue1 = dispatch_queue_create("com.example.gcd.MySerialDispatchQueue1", NULL);
dispatch_queue_t mySerialDispatchQueue2 = dispatch_queue_create("com.example.gcd.MySerialDispatchQueue2", NULL);
dispatch_queue_t targetDispatchQueue = dispatch_queue_create("com.example.gcd.TargetDispatchQueue", NULL);

dispatch_set_target_queue(mySerialDispatchQueue1, targetDispatchQueue);
dispatch_set_target_queue(mySerialDispatchQueue2, targetDispatchQueue);

dispatch_async(mySerialDispatchQueue2, ^{
    NSLog(@"1 %@", [NSThread currentThread]);
});
dispatch_async(mySerialDispatchQueue1, ^{
    NSLog(@"2 %@", [NSThread currentThread]);
});
dispatch_async(mySerialDispatchQueue2, ^{
    NSLog(@"3 %@", [NSThread currentThread]);
});
```

执行结果：

```
1 targetDispatchQueue所在的线程
3 targetDispatchQueue所在的线程
2 targetDispatchQueue所在的线程
```

注意即使任务 3 是在任务 2 之后添加，但是却比任务 2 先执行

以上这个例子中，队列 1 和队列 2 的目标队列都是一个串行队列，主线程执行完毕后，队列 2 有任务 1 和任务 3，队列 1 有任务 2。GCD 开始会优先处理了队列 2 的任务 1，继而继续处理了任务 3；处理完队列 2 之后再处理队列 1 的任务 2

### 如何利用 GCD 实现一个优先队列
```objc
dispatch_queue_t low = dispatch_queue_create("low",DISPATCH_QUEUE_SERIAL);
dispatch_queue_t high = dispatch_queue_create("high",DISPATCH_QUEUE_SERIAL);
// low 队列的目标队列指定为 high
dispatch_set_target_queue(low, high);
// 执行一个 low 任务
dispatch_async(low,^{
    NSLog(@"Low");
});

// 要分派到高优先级队列，暂停低优先级队列，并且在高优先级块结束后恢复低优先级队列：
dispatch_suspend(low);
dispatch_async(high,^{
    NSLog(@"High1");
    dispatch_resume(low);
});

dispatch_suspend(low);
dispatch_async(high,^{
    NSLog(@"High2");
    dispatch_resume(low);
});
```

`dispatch_set_target_queue(low, high)` 可以实现将所有的 low 队列的任务全部移到 high 队列处理，因此可以保证，处理 low 队列任务的时候，high 队列中现存的任务一定是优先于 low 队列。解决的是执行 low 的时候，high 的存量问题，保证了 low 一定要排队尾。

`dispatch_suspend` 保证添加 high 任务时，会暂停 low 队列，直到该任务完成才恢复。解决的是 high 任务的增量问题，保证了 high 可以插队


## 参考资料
+ [如何利用 GCD 实现一个优先队列](https://www.jianshu.com/p/20bffaa5526b)