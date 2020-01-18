## super 的本质
[《Objective-C Runtime初探：self super》](https://www.cnblogs.com/chenyg32/p/4811516.html)

```objc
@interface A : NSObject
- (void)f;
@end

@interface B : A
- (void)f;
- (void)g;
@end

@interface C : B
- (void)f;
@end
```

```objc
@implementation A
- (void)f
{
    NSLog(@"A");
}
@end


@implementation B
- (void)f
{
    NSLog(@"B");
}

- (void)g
{
    [self f];
    [super f];
    NSLog(@"%@", [self class]);
    NSLog(@"%@", [super class]);
}
@end

@implementation C
- (void)f
{
    NSLog(@"C");
}
@end
```

请问，下面代码输出什么？

```objc
C *c = [[C alloc] init];
[c g];
```

答案：CACC

## isKindOfClass、isMemberOfClass
[Runtime 源码](https://opensource.apple.com/source/objc4/objc4-647/runtime/NSObject.mm)

```objc
// isKindOf 和 isMemberOf 的基本用法和区别
Person *person = [[Person alloc] init];
BOOL res1 = [person isKindOfClass:[Person class]];              // YES
BOOL res2 = [person isMemberOfClass:[Person class]];            // YES
BOOL res3 = [person isKindOfClass:[NSObject class]];            // YES
BOOL res4 = [person isMemberOfClass:[NSObject class]];          // NO

// 由于类也是对象，所以 res5 和 res6
BOOL res5 = [[Person class] isKindOfClass:[NSObject class]];    // YES
BOOL res6 = [[Person class] isMemberOfClass:[NSObject class]];  // NO

BOOL res7 = [[NSObject class] isKindOfClass:[NSObject class]];  // YES
BOOL res8 = [[NSObject class] isMemberOfClass:[NSObject class]];// NO

BOOL res9 = [[Person class] isKindOfClass:[Person class]];      // NO
BOOL res10 = [[Person class] isMemberOfClass:[Person class]];   // NO
```

其中 res1-res4 展示了 isKindOf 和 isMemberOf 的基本用法和区别；res5、res6 展示了调用者是类对象时也是同理

按照我们的理解，isKindOf/isMemberOf 的调用者是实例对象时，参数应该是类对象；调用者是类对象时，参数应该是元类才比较合理

所以 res7 的输出有点奇怪，而 res8 和 res10 的输出也正常，然而 res9 却和 res7 不一致

我们看下源码

```objc
// 实例方法中的 self 是对象，[self class] 取得的是类对象，所以 cls 只有可能是类对象才有可能相等
- (BOOL)isMemberOfClass:(Class)cls
{
    return [self class] == cls;
}

// 类方法中的 self 是类对象，object_getClass 取得的是类对象的isa指针指向的对象，也就是元类对象，所以 cls 只有是元类对象才有可能相等
+ (BOOL)isMemberOfClass:(Class)cls
{
    return object_getClass((id)self) == cls;
}

// 实例方法中的 self 是对象，[self class] 取得的是类对象
// 从当前的类对象开始，向其父类方向查找，直到找到相等或尽头
- (BOOL)isKindOfClass:(Class)cls
{
	for (Class tcls = [self class]; tcls; tcls = tcls->superclass)
	{
		if (tcls == cls) return YES;
	}
	return NO;
}

// 类方法中的 self 是类对象，object_getClass 取得的是类对象的isa指针指向的对象，也就是元类对象
// 从当前的元类对象开始，向其父类方向查找，直到找到相等或尽头
+ (BOOL)isKindOfClass:(Class)cls
{
    for (Class tcls = object_getClass((id)self); tcls; tcls = tcls->superclass)
    {
        if (tcls == cls) return YES;
    }
    return NO;
}
```

关于 res7，当调用者是 [NSObject class] 时，调用的是 `+ (BOOL)isKindOfClass:(Class)cls`；self 是 NSObject 的类对象，`object_getClass` 返回的是 NSObject 元类，而传入的 cls 是 NSObject 类对象。

1. 第一次判断的时候，比较的对象是 NSObject 元类和 NSObject 类对象，不相等；
2. 第二次判断的时候，取 NSObject 元类的父类，即 NSObject 类对象，与 cls 比较，相等，返回 YES

关于 res9，第一次比较的对象 Person 元类与 Person 类对象；第二次比较的是 NSObject 元类与 Person 类对象；第三次比较的是 NSObject 类对象与 Person 类对象；结束，返回 NO

关于 res8，当调用者是 [NSObject class] 时，调用的是 `+ (BOOL)isMemberOfClass:(Class)cls`；self 是 NSObject 的类对象，`object_getClass` 返回的是 NSObject 元类，而传入的 cls 是 NSObject 类对象。不相等，返回 NO

res10 与 res8 同理


## 关于消息发送的 Crash
```objc
SEL commonReceiveSEL = @selector(didReceiveBaseEvent:);

if ([myDelegate respondsToSelector:commonReceiveSEL])
{
    void (*commonEventIMP)(id, SEL, id) = (void (*)(id, SEL, id))[self methodForSelector:];
    commonReceiveIMP(myDelegate, commonReceiveSEL, nil);
}
```

这个例子中，self 没有实现这个 didReceiveBaseEvent:，应该把 self 改为 myDelegate

但是执行 commonReceiveIMP 时，居然 myDelegate 的方法被正确调用了，正常情况应该是抛错误：`doesNotRecognizeSelector:`

methodForSelector 返回的是方法的 IMP，如果找不到该 IMP，则会返回 `_objc_msgforward` 这个 IMP

`_objc_msgforward` 调用之后会进入 Runtime 的消息转发流程

项目中有一个 SDK hook 了消息转发流程的第三个阶段，即 `methodSignatureForSelector` 和 `forwardInvocation:`，使得 commonReceiveSEL 转发给了 myDelegate

当把这个 SDK 移除之后，果然就抛了 `doesNotRecognizeSelector:` 的错误

相关面试题：`_objc_msgForward` 是什么


## 类方法中的 self v.s. 实例方法中的 self 和 [self class]


## Runtime 如何实现 weak 属性
[weak 的实现原理](../weak 的实现原理/)