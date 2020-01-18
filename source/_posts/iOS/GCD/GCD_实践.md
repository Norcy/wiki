## 1. 异步处理耗时任务后主线程更新 UI

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

## 2. RunLoop

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
	
输出 1、3，因为子线程没有开启 RunLoop

## 3. 如何实现多读单写？

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
    dispatch_barrier_async(self.concurrentQueue, ^{
        // 写操作
        [self.dic setObject:object forKey:key];
    });
}
```
	
注意 `__block` 的使用

## 4. 如何异步下载多张图片后并拼接

见 `dispatch_group_notify`
	
如果需要按序下载图片呢？则使用串行队列即可

## 5. 苹果为什么要废弃 `dispatch_get_current_queue`
	
未完全搞明白_Todo
	
[解答](https://www.cnblogs.com/chmhml/p/7421050.html)
	
进阶：怎么判断当前队列是指定队列？
	
	
	
