## 异步处理耗时任务后主线程更新 UI

```objc
// 获取全局并发队列
dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0); 
dispatch_async(queue, ^{
    // 耗时操作
    dispatch_async(dispatch_get_main_queue(), ^{
	    // 回主线程刷 UI
    });
});
```

## 输出顺序

```objc
- (void)viewDidLoad
{
    [super viewDidLoad];
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    dispatch_async(queue, ^{
        NSLog(@"1");
        [self performSelector:@selector(printLog) withObject:nil afterDelay:0];
        NSLog(@"3");
    });
}
	
- (void)printLog
{
    NSLog(@"2");
}
```
	
输出 1、3，因为子线程没有开启 RunLoop；注意即使是 afterDelay 为 0，也是会放到下个 RunLoop 去做

## 如何实现多读单写？

```objc
- (id)objectForKey:(NSString *)key
{
    __block id ret;	// 注意 __block
    dispatch_sync(self.concurrentQueue, ^{
        // 读操作
        ret = [self.dic objectForKey:key];
    });
    return ret;
}

- (void)setObject:(id)object forKey:(NSString *)key
{
    key = [key copy];
    dispatch_barrier_async(self.concurrentQueue, ^{
        // 写操作
        [self.dic setObject:object forKey:key];
    });
}
```
	
1. 注意 `__block` 的使用
2. 注意读的时候是同步；写的时候是异步。我们不需要使每次程序执行的时候都等待写操作完成，所以写操作异步执行，但是我们需要同步的执行读操作来保证程序能够立刻得到它想要的值
3. 使用的队列是自己创建的而不是全局并发队列，因为我们不想写操作的时候阻塞全局队列的其他任务，我们只希望在写的同时，不会有其他的写操作或者读操作
4. `dispatch_barrier_async` 的 block 运行时机是，在它之前所有的任务执行完毕，并且在它后面的任务开始之前，期间不会有其他的任务执行
5. 写操作的时候需要对 key 进行 copy，防止传入的 key 是 NSMutableString，异步之后它被外部修改

## MRC 下的多读单写
如果把 ARC 的代码转成以下 MRC 会有什么问题

```objc
- (id)objectForKey:(NSString *)key
{
    __block id ret;
    dispatch_sync(self.concurrentQueue, ^{
        ret = [self.dic objectForKey:key];
    });
    return ret;
}
    
- (void)setObject:(id)object forKey:(NSString *)key
{
    key = [key copy];
    dispatch_barrier_async(self.concurrentQueue, ^{
        [self.dic setObject:object forKey:key];
    });
    [key release];  // MRC 添加
}
```

这样如果 A 线程正读取完一个值；而 B 线程随后对该值进行置 nil，那么 A 拿到的数据就会野指针

解决方案如下：

```objc
- (id)objectForKey:(NSString *)key
{
    __block id ret;
    dispatch_sync(self.concurrentQueue, ^{
        ret = [[self.dic objectForKey:key] retain];
    });
    return [ret autorelease];
}
```


## 如何异步下载多张图片后并拼接

见 `dispatch_group_notify`
	
如果需要按序下载图片呢？则使用串行队列即可