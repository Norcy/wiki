启动优化的一个重点就是将启动任务放到子线程异步处理，因此遇到很多多线程安全问题，这里我们将这些问题记录下来

# 子线程与延时调用
在子线程中进行延时调用，比如调用`performSelector:withObject:afterDelay:`和创建 NSTimer 都会失效，因为GCD开的子线程没有启用 RunLoop，线程刚创建完就被销毁，所以延时就失效了。解决方法是利用 dispatch_async 到主线程进行延时调用或创建 NSTimer。详见下面代码

```objc
dispatch_async(self.dispatch_queue, ^{
    // 1 invalid timer
    [self performSelector:@selector(sayHello) withObject:nil afterDelay:2.f];

    // 2 valid timer
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2.f * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self sayHello];
    });

    // 3 invalid timer
    [NSTimer scheduledTimerWithTimeInterval:2.f target:self selector:@selector(sayHello) userInfo:nil repeats:YES];

    // 4 valid timer
    dispatch_async(dispatch_get_main_queue(), ^{
        [NSTimer scheduledTimerWithTimeInterval:2.f target:self selector:@selector(sayHello) userInfo:nil repeats:YES];
    });
});
```
# NSMutableArray 与 NSArray 的线程安全
现在我们用 NSMutableArray 与 NSArray 这2个例子来说明多线程中2种常见的线程安全问题

## 1. NSMutableArray 是线程不安全的，NSArray 是线程安全的
我们常说，NSMutableArray 是线程不安全的，NSArray 是线程安全的。这句话是什么意思呢？

其实，这里说的线程安全或线程不安全，指的是__对其属性的访问或方法的调用__，在多线程下的安全性

NSMutableArray 的不安全在于，NSMutableArray 内部是不加锁的，所以对其同时进行增删改查是不安全的

举个例子，一个线程调用其 addObject，另一个线程调用 removeObject/count/遍历等，可能会 crash

而 NSArray 相对来说是安全的，因为它是不可变对象，无法增删改，所以在多线程下不会发生跟 NSMutableArray 一样的安全问题

这就是 NSMutableArray 是线程不安全的，NSArray 是线程安全的，这句话的意思

## 2. 如何封装一个线程安全的容器类（NSMutableArray）
那么 NSMutableArray 的安全性问题如何解决呢？可以通过加锁来保护，加锁保护包括 `@synchronized`、`NSLock`、`concurrentQueue+dispatch_sync+dispatch_barrier_async` 等等

QLTaskQueue 展示了如何对 NSMutableArray 进行线程安全的封装

QLTaskQueue.h 

```objc
@class QLTaskModel;

@interface QLTaskQueue : NSObject

- (NSArray *)tasks;
- (NSInteger)taskCount;
- (void)addTask:(QLTaskModel *)model;
- (void)removeTask:(QLTaskModel *)model;
- (QLTaskModel *)taskModel:(NSInteger)index;

@end
```

QLTaskQueue.m

```objc
#import "QLTaskQueue.h"
#import "QLTaskModel.h"

@interface QLTaskQueue ()
@property (nonatomic, strong) NSMutableArray *taskList;
@end

@implementation QLTaskQueue

- (id)init
{
	if (self = [super init])
	{
		self.taskList = [NSMutableArray array];
	}
	return self;
}

- (NSArray *)tasks
{
    @synchronized(self.taskList)
    {
        // 返回TaskList的复制，否则多线程下 QLTaskQueue 对数组增删元素时，外部可能对该数组进行遍历等操作导致多线程安全问题
        return [self.taskList copy];
    }
}

- (NSInteger)taskCount
{
    @synchronized(self.taskList)
    {
        return [self.taskList count];
    }
}

- (void)addTask:(QLTaskModel *)model
{
	@synchronized(self.taskList)
	{
		[self.taskList addObject:model];
	}
}

- (void)removeTask:(QLTaskModel *)model
{
	@synchronized(self.taskList)
	{
		[self.taskList removeObject:model];
	}
}

- (QLTaskModel *)taskModel:(NSInteger)index
{
	@synchronized(self.taskList)
	{
		if (index < [self.taskList count])
		{
			return [self.taskList objectAtIndex:index];
		}
	}
	return nil;
}

@end
```

主要注意两点：

1. 对 NSMutableArray 的访问都要加锁保护
2. 返回 NSMutableArray 的接口一定要返回其 copy，否则多线程下 QLTaskQueue 对数组增删元素时，外部可能对该数组进行遍历等操作导致多线程安全问题

关于返回 copy

再延伸一下，假如 ClassA 中有以下方法

```objc
// ClassA

static NSString *str = nil;

+ (NSString *)getStr
{
    str = @"123";
    return str;
}
```

假如外部不是用 copy 属性的 NSString 接住这个返回值，比如

```objc
NSString *localStr = [[self class] getStr];
[localStr stringByAppendingString:@"456"];
```

那么多线程环境下，这种是会 crash 的，解决方法就是改为

```objc
+ (NSString *)getStr
{
    str = @"123";
    return [str copy];
}
```

## 3. 是否多线程下，NSArray 就绝对安全？
NSArray 相对 NSMutableArray 来说确实是安全了一些，但在多线程下并不是绝对安全的。看一个简单的例子

```objc
@property (nonatomic, copy) NSArray *array;

- (void)writeDirectlyForNSArray
{
	self.array = @[ @(1), @(2), @(3) ];
}

- (void)readDirectlyForNSArray
{
	self.array; // do nothing
}
```

如果线程1循环调用方法1，而线程2循环调用方法2，会发现是有 crash 的

因为 array 是 nonatomic，这个意味着 array 的 setter&getter 方法都不是原子操作，多线程下进行 setter&getter，会导致读写冲突

不止 NSArray，只要是 oc 对象，都有可能发生类似的安全性问题，包括 NSMutableArray、NSString、NSDictionary 等等

在解决线上版本 crash 过程中，我们发现类似 NSArray 的线程不安全问题往往更常见，因为这类读写冲突问题更隐蔽

解决方法很简单，只要将 nonatomic 属性改为 atomic 即可

关于 atomic 和 nonatomic，可以延伸阅读下[iOS多线程到底不安全在哪里？](http://mrpeak.cn/blog/ios-thread-safety/)

## 4. 一个 demo，更深入理解多线程安全问题
写了一个 demo，[ThreadSafeDemo](https://github.com/Norcy/ThreadSafeDemo)，主要做了以下这些

+ 模拟了多线程并发环境
+ 模拟 NSArray 和 NSMutableArray 在多线程下的 crash
+ 给出解决方法：NSArray 改为 atomic，NSMutableArray 则进行加锁（`@synchronized`、`NSLock`、`concurrentQueue+dispatch_sync+dispatch_barrier_async`等）

`concurrentQueue+dispatch_sync+dispatch_barrier_async` 是一个经典的用法，效率还不错，可以延伸阅读下[锁、dispatch_queue的性能](http://www.jianshu.com/p/ae1cef1f3187)

# 单例
篇幅受限，放到[《iOS的单例模式与多线程安全》](http://norcy.github.io/2017/04/05/iOS%E7%9A%84%E5%8D%95%E4%BE%8B%E6%A8%A1%E5%BC%8F%E4%B8%8E%E5%A4%9A%E7%BA%BF%E7%A8%8B%E5%AE%89%E5%85%A8/)

# Lock
+ [锁的性能](http://www.jianshu.com/p/ae1cef1f3187)：synchronized 适用于低频获取/释放锁，且获取锁后执行代码片段较少的场景。
+ @synchronize 与字符串：可以用字符串来作为 @synchronize 的锁标识
+ NSLock 和 NSRecursiveLock 的 lock 和 unlock 操作必须在同一线程，不然会有意想不到的后果
+ NSLock 在同一线程重复进行 lock 会导致死锁，解决此问题可以使用递归锁；递归锁的设计初衷就是用来解决，同一线程内，对 NSLock 进行重复 lock 会导致死锁的问题
+ NSLock 和 NSRecursiveLock 的 tryLock 方法不靠谱，假如线程 A 加锁再解锁，然后线程 A 的 tryLock 返回 YES，这是对的，但是线程 B 去 tryLock 却是返回 NO，无法理解 tryLock 的用法，感觉 tryLock 跟 lock、unlock 一样，都得在同一线程使用

# 自增操作原子化
使用 `OSAtomicIncrement32(&i);` 代替 `i++`，才能线程安全（需要 `#import <libkern/OSAtomic.h>`）

# 如何判断一段代码是线程安全？
+ 单例的初始化方法中，对自身属性的访问都是线程安全的
+ 对常用类的线程安全性要敏感（比如 NSMutableArray、NSMutableDictionary 不是线程安全，NSFileManager、NSUserDefaults 是线程安全）

# 常用类的线程安全
常用类的线程安全可以从[苹果官方文档](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/Multithreading/ThreadSafetySummary/ThreadSafetySummary.html)查阅，除了苹果这份文档，有些常见类也需要记住，比如

1. NSCache 是线程安全的，From 苹果官方：
    > You can add, remove, and query items in the cache from different threads without having to lock the cache yourself.

2. NSOperationQueue 的增删任务是线程安全，From [苹果官方](https://developer.apple.com/reference/foundation/operationqueue)
    > It is safe to use a single NSOperationQueue object from multiple threads without creating additional locks to synchronize access to that object.


# 干货推荐
+ [iOS多线程到底不安全在哪里？](http://mrpeak.cn/blog/ios-thread-safety/)
+ [锁、dispatch_queue的性能](http://www.jianshu.com/p/ae1cef1f3187)
+ [如何看待多线程](https://gist.github.com/xiayun200825/eeccfd31ba13aca3eb9e3c191ac724dd)