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