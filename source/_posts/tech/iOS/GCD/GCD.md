
+ [GCD 脑图](http://naotu.baidu.com/file/9879bff74c52d54d1248714530fdd84a)

# GCD 队列和任务
## 队列（Dispatch Queue）
+ 串行队列：只开启一个线程，一个任务执行完毕后，再执行下一个任务
+ 并发队列：可以开启多个线程，并且同时执行任务

## 任务
执行任务的两种方式：

+ 同步执行
	+ 会卡当前线程
	+ 不具备开启新线程的能力

+ 异步执行
	+ 异步添加任务到指定的队列中，不卡当前线程
	+ 具备开启新线程的能力（有能力不代表一定会开启新线程）


# 队列的创建/获取
## `dispatch_queue_create`
```objc
// 串行队列的创建方法
dispatch_queue_t queue = dispatch_queue_create("com.norcy.serial", DISPATCH_QUEUE_SERIAL);
// 并发队列的创建方法
dispatch_queue_t queue = dispatch_queue_create("com.norcy.concurrent", DISPATCH_QUEUE_CONCURRENT);
```

第一个参数是线程名称，推荐使用__逆序全程域名__，用于调试

第二个参数用于区别串行或并发，串行时填 `DISPATCH_QUEUE_SERIAL` 或者 NULL 都可以，并发填 `DISPATCH_QUEUE_CONCURRENT`

1. 串行队列

	可以使用 `dispatch_queue_create` 创建多个串行队列，它们之间将互不影响，可以并行执行。每创建一个串行队列，系统就会生成一个线程，如果线程数过多，就会引发大量的上下文切换降低性能

	串行队列的使用场景：多个线程更新相同资源导致数据竞争时使用
	
2. 并发队列

	不管生成多少并发队列，系统内核只会使用有效管理的线程，不会出现串行队列的线程过多的问题
	
3. 内存管理

	即使在 ARC 下，每次使用 `dispatch_queue_create` 都得配套使用 `dispatch_release` 来释放
	
	```objc
	dispatch_queue_t myConcurrentDispatchQueue = dispatch_queue_create("com.example.gcd.MyConcurrentDispatchQueue", DISPATCH_QUEUE_CONCURRENT);
 
	dispatch_async(queue, ^{NSLog(@"block on myConcurrentDispatchQueue");});
	 
	dispatch_release(myConcurrentDispatchQueue);
	```
	
	以上代码不会出现问题，因为 Block 拷贝的时候会对 Dispatch Queue 执行 `dispatch_retain` 操作，在 Block 结束时会执行 `dispatch_release` 操作

4. 优先级

	无论是创建串行还是并发队列，其优先级都是 `DISPATCH_QUEUE_PRIORITY_DEFAULT`
	
## 主队列
对于串行队列，GCD 提供了的一种特殊的串行队列：主队列（Main Dispatch Queue）

+ 所有放在主队列中的任务，都会放到主线程中执行
+ 通过 `dispatch_get_main_queue()` 获取

## 全局并发队列
对于并发队列，GCD 默认提供了全局并发队列（Global Dispatch Queue）

可以使用 `dispatch_get_global_queue` 来获取。需要传入两个参数：第一个参数表示队列优先级。第二个参数暂时没用，传 0 即可

优先级有 4 种：High > Default > Low > Background

```objc
// 全局并发队列的获取方法
dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
```

# 队列和任务的组合使用
+ 同步执行一定不能开新线程
+ 异步执行具备开启新线程的能力
+ 串行队列最多只能开启一个线程
+ 并行队列可开启多个线程

## 串行队列+同步执行
会卡住当前线程，任务在当前线程顺序执行

```
cur start 
cur 1
cur 2
cur 3
cur end
```

## 串行队列+异步执行
不会卡住当前线程，开1个新线程，任务在新线程顺序执行

```objc
cur start 
cur end
new 1
new 2
new 3
```

## 并发队列+同步执行
会卡住当前线程，任务在当前线程顺序执行

```
cur start 
cur 1
cur 2
cur 3
cur end
```

## 并发队列+异步执行
不会卡住当前线程，开3个新线程，任务在新线程并发执行

```
cur start
cur end
new2 2
new1 1
new3 3
```

## 主队列+同步执行
+ 如果当前是在主线程，则会死锁。主线程等待任务完成，而任务等待主线程
+ 如果当前是在子线程，则会卡住当前线程，任务在主线程顺序执行

串行队列+同步执行，如果当前所在队列和目标队列是同一个，那么就会死锁


## 主队列+异步执行
不会卡住当前线程，任务在主线程顺序执行

# 常用 API
## `dispatch_set_target_queue`
见 [dispatch_set_target_queue](../dispatch_set_target_queue)

## `dispatch_after`
注意 `dispatch_after` 并不是再指定时间后执行处理，而只是在指定时间追加任务到 Dispatch Queue

因为主队列在主线程的 RunLoop 中执行，比如该 RunLoop 每 1/60 秒执行一次，则 Block 最快在 3 秒后执行，最慢在 3+1/60 秒后执行，并且如果队列中有其他任务影响，则这个时间会功更长

## `dispatch_group`
1. `dispatch_group_notify`
	
	使用 `dispatch_group_async` 添加任务，一旦检测到所有任务执行完毕，`dispatch_group_notify` 中的任务就被会追加到指定队列中并处理
	
	经典场景：下载多张图片，等它们都下载后，再合成一张图片
	
	```objc
	dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	dispatch_group_t group = dispatch_group_create();
	dispatch_group_async(group, queue, ^{ /*加载图片1 */ });
	dispatch_group_async(group, queue, ^{ /*加载图片2 */ });
	dispatch_group_async(group, queue, ^{ /*加载图片3 */ }); 
	dispatch_group_notify(group, dispatch_get_main_queue(), ^{
	    // 合并图片
	});
	```

2. `dispatch_group_wait`
	
	使用 `dispatch_group_wait` 可以阻塞当前线程，等待 Group 中的任务执行完成之后，才会解除阻塞
	
	等待无限长时间，直到 Group 中的所有任务完成
	
	```objc
	dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	dispatch_group_t group = dispatch_group_create();
	dispatch_group_async(group, queue, ^{ /*加载图片1 */ });
	dispatch_group_async(group, queue, ^{ /*加载图片2 */ });
	dispatch_group_async(group, queue, ^{ /*加载图片3 */ }); 
	dispatch_group_wait(group, DISPATCH_TIME_FOREVER);    
	// 合并图片
	```
	
	等待 5 秒，如果 5 秒后没完成或者 5 秒内完成，都会解除阻塞，继续执行；返回的 long 值为 0 则全部任务执行完毕，反之没完成
	
	```objc
	dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	dispatch_group_t group = dispatch_group_create();
	dispatch_group_async(group, queue, ^{ /*加载图片1 */ });
	dispatch_group_async(group, queue, ^{ /*加载图片2 */ });
	dispatch_group_async(group, queue, ^{ /*加载图片3 */ }); 
	long result = dispatch_group_wait(group, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));   
	if (result == 0)
	{
		// 成功，合并图片 
	}
	else
	{
		// 全部任务未执行完毕
	}
	```

## `dispatch_barrier_async`
`dispatch_barrier_async` 会在两个操作组间形成栅栏，在执行完栅栏前面的操作之后，才执行栅栏操作，最后再执行栅栏后边的操作

为了防止读写冲突，采取同步读取，异步写入，此时就可以使用该方法

```objc
dispatch_queue_t queue = dispatch_queue_create("com.example.forBarrier", DISPATCH_QUEUE_CONCURRENT);
dispatch_async(queue, reading1);
dispatch_async(queue, reading2);
dispatch_barrier_async(queue, writing1);
dispatch_barrier_async(queue, writing2);
dispatch_async(queue, reading3);
dispatch_async(queue, reading4);
```

执行结果可能为：reading2 -> reading1 -> writing1 -> writing2 -> reading3 -> reading4

writing 之前添加的 reading 任务顺序是不确定的；writing 一定会等到之前添加的所有 reading 任务结束之后才执行；writing 之后添加的任务一定会在 writing 结束之后再执行，如下图

![](https://p1-jj.byteimg.com/tos-cn-i-t2oaga2asx/gold-user-assets/2018/2/24/161c6258a0c04a71~tplv-t2oaga2asx-zoom-in-crop-mark:3024:0:0:0.awebp)

`dispatch_barrier_sync` 与 `dispatch_barrier_async` 的区别在于，会等待其队列中的任务执行完毕再返回

writing1 一定先于 writing2，如果是两个写任务并发了，`dispatch_barrier_async` 也会保证同时只有一个生效，顺序是根据进入栅栏时的顺序决定的，并非由并发队列决定

注意 `dispatch_barrier_async`` 必须配合自定义的并发队列使用，才能达到写互斥的效果，如下例

```objc
// dispatch_barrier_async 必须配合自定义的并发队列使用，才能达到写互斥的效果，以下输出是按顺序的
dispatch_queue_t queue1 = dispatch_queue_create("com.example.forBarrier", DISPATCH_QUEUE_CONCURRENT);
// 如果搭配全局并发队列，会退化为 dispatch_async，导致任务的执行顺序是不确定的，以下输出是不按顺序的
dispatch_queue_t queue2 = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
int i = 9999;
while (i--) {
		dispatch_barrier_async(queue1, ^{
				NSLog(@"%@", @(i));
		});
}```

## `dispatch_apply`
用于重复执行任务，类似 for 循环，参数 Block 是带有 index 参数信息的，注意是同步执行的

```objc
dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
NSLog(@"begin");
dispatch_apply(3, queue, ^(size_t index) {
    NSLog(@"%@",index, [NSThread currentThread]);
});
NSLog(@"end");
```

输出结果为

```
begin
2
1
3
end
```

可以看到，index 的运用和同步执行的性质


## `dispatch_suspend` 和 `dispatch_resume`
```objc
dispatch_suspend(queue);
dispatch_resume(queue);
```

挂起后，已经执行过的任务不会受到影响，而已经追加到队列中但尚未执行的任务将停止执行，直到队列被恢复

什么场景下使用？

+ 一般在内存警告后取消队列中的操作
+ 为了保证 scorllView 在滚动的时候流畅，通常在滚动开始时，暂停队列中的所有操作，滚动结束后，恢复操作


## `dispatch_group_enter` 和 `dispatch_group_leave`
假如有一个耗时操作 A 和 2 个网络请求 B、C，现在需要等到 ABC 都完成后刷新页面

注意如果网络请求的代码不使用 `dispatch_group_enter` 而使用 `dispatch_group_async` 是有问题的

```objc
// 错误的示例
// B 网络请求
dispatch_group_async(group, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
    [self sendRequestWithCompletion:^(id response) {
        number += 2;
    }];
});
```

问题在于，加入队列中的任务是一个网络请求，它是异步的，因此这个任务会被立刻完成（尽管此时网络请求还没回来）

对于 **group 多任务中的异步任务，我们需要使用 `dispatch_group_enter` 和 `dispatch_group_leave` 实现**

```objc
- (void)testGCDEnter
{
	dispatch_group_t group = dispatch_group_create();

	__block NSInteger number = 0;

	// A 耗时操作
	dispatch_group_async(group, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
	    sleep(3);
	    number += 1;
	});

	// B 网络请求
	dispatch_group_enter(group);
	[self sendRequestWithCompletion:^(id response) {
	    number += 2;
	    dispatch_group_leave(group);
	}];

	// C 网络请求
	dispatch_group_enter(group);
	[self sendRequestWithCompletion:^(id response) {
	    number += 3;
	    dispatch_group_leave(group);
	}];

	dispatch_group_notify(group, dispatch_get_main_queue(), ^{
	    NSLog(@"%zd", number);
	});
}

- (void)sendRequestWithCompletion:(void (^)(id response))completion
{
    //模拟一个网络请求
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        sleep(1);
        dispatch_async(dispatch_get_main_queue(), ^{
            if (completion) completion(nil);
        });
    });
}
```

当调用 `dispatch_group_create` 的时候，生成的 group 内部会维护一个信号量。

+ 当调用 `dispatch_group_enter` 的时候，信号量会减 1；
+ 当调用 `dispatch_group_leave` 的时候，信号量会加 1；此时会判断信号量是否恢复为初始值；如果是则调用 `dispatch_group_notify`
+ 当调用 `dispatch_group_async` 的时候，其内部实际上也调用了 `dispatch_group_enter` 和 `dispatch_group_leave`

注意：

1. `dispatch_group_enter` 必须在 `dispatch_group_leave` 之前出现
2. `dispatch_group_enter` 和 `dispatch_group_leave` 必须成对出现

更多资料可参考：[深入理解GCD之dispatch_group](https://juejin.im/post/5c761be8f265da2d993d9578)


## `dispatch_once`
单例

## `dispatch_semaphore`
少用，待补充
## dispatch iO
少用，待补充
	
# 相关链接
+ [我的脑图](http://naotu.baidu.com/file/9879bff74c52d54d1248714530fdd84a?token=493b9c759767a3b8)
+ [iOS多线程：『GCD』详尽总结](https://juejin.im/post/5a90de68f265da4e9b592b40)


