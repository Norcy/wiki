## autorelease 的伪代码实现

```objc
[obj autorelease];
```

等同于以下代码

```objc
// NSObject.m:
- (id)autorelease 
{
    [NSAutoreleasePool addObject:self];
}
```

```objc
// NSAutoreleasePool. 类方法
+ (void)addObject:(id)anObj 
{
    NSAutoreleasePool *pool = 获取正在使用中的 pool；
    if (pool != nil) 
    {
        [pool addObject:anObj];
    }
}

// NSAutoreleasePool.m 实例方法
- (void)addObject:(id)anObj 
{
    [array addObject:anObj];
}
```

其中关于获取正在使用中的 pool，以下例子的 obj 在调用 autorelease 时取到的 autoreleasepool 就是 poo2

```objc
NSAutoreleasePool *pool1 = [[NSAutoreleasePool alloc] init];
NSAutoreleasePool *pool2 = [[NSAutoreleasePool alloc] init];

NSObject *obj = [[NSObject alloc] init];
[obj autorelease]

[poo1 drain];
[poo2 drain];
```


```objc
// drain 的实现
- (void)drain
{
    [self dealloc];
}

- (void)dealloc
{
    [self emptyPool];
    [array release];
}

- (void)emptyPool
{
    for (id obj in array)
    {
        [obj release];
    }
}
```


## autorelease 苹果的实现
```objc
class AutoreleasePoolPage 
{
    static inline void *push() 
    {
        // 相当于生成或持有 NSAutoreleasePool 类对象
    }

    static inline id autorelease(id obj)
    {
        // 相当于 NSAutoreleasePool 类的 addObject 类方法
        AutoreleasePoolPage *autoreleasePoolPage = 取得正在使用的 AutoreleasePoolPage 实例；
        autoreleasePoolPage->add(obj);
    }
    id *add(id obj) 
    {
        // 将对象追加到内部数组中
    }

    static inline void *pop(void *token)
    {
        // 相当于废弃 NSAutoreleasePool 类对象
        releaseAll();
    }
    void releaseAll() 
    {
        // 调用内部数组中对象的 release 实例方法
    }
};
void *objc_autoreleasePoolPush(void)
{
    return AutoreleasePoolPage::push();
}
void objc_autoreleasePoolPop(void *ctxt)
{
    AutoreleasePoolPage::pop(ctxt);
}
id *objc_autorelease(id obj) 
{
    return AutoreleasePoolPage::autorelease(obj);
}
```

```objc
// 等同于 objc_autoreleasePoolPush()
NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

id obj = [[NSObject alloc] init];
// 等同于 objc_autorelease(obj)
[obj autorelease];

// 等同于 obic_autoreleasePoolPop(pool)
[pool drain];
```


## AutoreleasePool 的底层实现
1. AutoreleasePool 是由一个个 AutoreleasePoolPage 组成的双向链表
2. AutoreleasePoolPage 内部维护一个栈；栈满的时候会新建一个 AutoreleasePoolPage 节点
3. AutoreleasePool Push 时会压入一个边界对象表示一个 AutoreleasePool 的开始，Pop 时会清理堆栈直到遇到一个边界对象；边界对象是界定 AutoreleasePool 的分割线

![](https://p1-jj.byteimg.com/tos-cn-i-t2oaga2asx/gold-user-assets/2018/5/23/1638c0ede96e603e~tplv-t2oaga2asx-zoom-in-crop-mark:3024:0:0:0.awebp)

## AutoreleasePool 与 RunLoop
App 启动后，苹果在主线程 RunLoop 里注册了两个 Observer，区别是优先级不同

第一个 Observer 优先级最高，保证创建释放池发生在其他所有回调之前，监视了一个事件：

+ Entry（即将进入 Loop），其回调内会调用 `_objc_autoreleasePoolPush()` 创建自动释放池。

第二个 Observer 优先级最低，保证其释放池子发生在其他所有回调之后，监视了两个事件：

+ BeforeWaiting（准备进入休眠）时调用 `_objc_autoreleasePoolPop()` 和 `_objc_autoreleasePoolPush()` 释放旧的池并创建新池；
+ Exit(即将退出 Loop) 时调用 `_objc_autoreleasePoolPop()` 来释放自动释放池。


在主线程执行的代码，通常是写在诸如事件回调、Timer 回调内的。这些回调会被 RunLoop 创建好的 AutoreleasePool 环绕着，所以不会出现内存泄漏，开发者也不必显示创建 Pool 了

## 参考文章
+ [AutoreleasePool 底层实现原理](https://juejin.im/post/5b052282f265da0b7156a2aa)