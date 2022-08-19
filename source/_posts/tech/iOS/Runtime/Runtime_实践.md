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

## NSObject 分类的 -test 与 Person 的无实现 +test
问：假如 Person 类声明了 `+ (void)test` 方法，但是没实现，而 NSObject 的分类实现了 `- (void)test` 方法；此时调用 Person 的 test 类方法，会发生什么事？

```objc
// Person.h
@interface Person : NSObject
+ (void)test;   // 没有实现
@end

// NSObject+Runtime.m
@implementation NSObject (Runtime)
- (void)test
{
    NSLog(@"NSObject's test");
}
@end

[[Person class] test];  // 会发生什么？
```

答：虽然 Person 没有实现这个类方法，但是最终会调用 NSObject 的 test 实例方法。

对于类方法，消息发送从 Person 类的 isa 指针指向的元类开始查找方法，沿着继承链向上查找，最终找到 NSObject 元类，找不到，继续查找 NSObject 的类对象，最终发现了 test 方法，进而进行调用

本题的考点是：

1. NSObject 元类的父类就是 NSObject 类对象
2. 查找方法的 Key 是 @selector，并不包括其是实例方法还是类方法；或者说实例方法和类方法的区别就在于查找的继承链，而不在于方法本身

## Runtime 怎么添加属性、方法
```objc
class_addIvar           // 添加成员变量
class_addMethod         // 添加方法
class_addProperty       // 添加属性
class_addProtocol       // 添加协议
class_replaceProperty   // 替换属性
```

需要注意的是，成员变量和属性的区别，详见 [方法交换](../%E6%96%B9%E6%B3%95%E4%BA%A4%E6%8D%A2/)

## 为什么 Category 中不能动态添加成员变量
因为 `category_t` 的结构中没有含有成员变量的字段，但是含有属性相关的字段，因此虽然无法动态添加成员变量，但是可以动态添加属性

详见：[如何为分类添加属性](../如何为分类添加属性/)

## 能否在分类中增加属性
不能添加成员变量；不能直接添加属性，但是可以通过 Runtime 的方法添加

因为方法和属性并不“属于”类实例，而成员变量“属于”类实例

因为 Category 在运行期，对象的内存布局已经确定，如果添加实例变量就会破坏类的内部布局。

详见：[如何为分类添加属性](../如何为分类添加属性/)

## 能否向编译后得到的类中增加实例变量？能否向运行时创建的类中添加实例变量？
不能向编译后得到的类中增加实例变量；能向运行时创建的类中添加实例变量；

添加实例变量会影响类结构体中的 `objc_ivar_list` 和 `instance_size`

因为编译后的类已经注册在 Runtime 中，类结构体中的 `objc_ivar_list` 实例变量的链表和 `instance_size` 实例变量的内存大小已经确定，同时 Runtime 会调用 `class_setIvarLayout` 或 `class_setWeakIvarLayout` 来处理 strong weak 引用，所以不能向存在的类中添加实例变量

运行时创建的类是可以添加实例变量，调用 `class_addIvar` 函数，但是得在调用 `objc_allocateClassPair` 之后，`objc_registerClassPair` 之前，原因同上。 

## 类和分类的同名方法
+ 如果分类中有和原有类同名的方法, 会优先调用分类中的方法，即 分类 > 原类
+ 如果多个分类中都有和原类同名的方法, 那么各个分类的方法调用顺序由编译器决定，即 分类1 or 分类2 > 原类

## 分类和扩展的区别
+ 扩展我们天天使用，既可以添加 @property（一般用来声明私有变量），也可以添加方法（但是没必要）
+ 分类只能添加方法，正确添加 @property 需要用 Runtime 的方法
+ 类扩展是在编译阶段被添加到类中，而类别是在运行时添加到类中

## 项目中用过 Runtime 吗
项目中用过的：

1. Swizzle Method（API 安全性保护，防止崩溃；AOP，VideoReport 的埋点上报、日志）
2. 给分类添加属性（给系统的类添加属性，如 VN 中为 UIView 添加 Cell 信息）
3. 打印类的所有属性值

自己没用过但是知道的：

1. Runtime 如何实现自定义 KVO
2. Runtime 如何实现 weak 属性
3. 实现 NSCoding 的自动归档和自动解档


## 简单描述下 Runtime 的消息机制
分为两个阶段：消息发送和消息转发

消息发送：

+ 当调用实例方法时：通过 **isa 指针**找到实例对应的类对象，并且在其中的**缓存方法列表**以及方法列表中进行查询，如果找不到则根据 **`super_class` 指针**在父类中查询，直至根类(NSObject 或 NSProxy)
+ 当调用类方法时：通过 isa 指针找到类对象对应的元类并且在其中的缓存方法列表以及方法列表中进行查询，如果找不到则根据 `super_class` 指针在父类中查询，直至根类(NSObject 或 NSProxy)

如果没有找到对应的 IMP，则进入消息转发流程：

1. 动态方法解析（`+resolveInstanceMethod:` 或者 `+resolveClassMethod:`），动态添加方法实现的机会
2. 快速转发（替换消息接收者）（`-forwardingTargetForSelector:`），替换消息的接收者为其他对象的机会
3. 完整消息转发（`-methodSignatureForSelector:` && `-forwardInvocation:`），可以实现多次转发，转发给多个对象，是第二阶段的扩充

## `_objc_msgForward` 是什么，直接调用它将会发生什么？
+ `_objc_msgForward` 是 IMP 类型，当向一个对象发送一条消息，会调用 methodForSelector 方法，但它并没有实现的时候，该方法会返回该 IMP，即 `_objc_msgForward`，这个 IMP 用于执行消息转发
+ 直接调用会进入消息转发的三个流程

### 工作遇到的例子
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

**`_objc_msgforward` 调用之后会进入 Runtime 的消息转发流程**

项目中有一个 SDK hook 了消息转发流程的第三个阶段，即 `methodSignatureForSelector` 和 `forwardInvocation:`，使得 commonReceiveSEL 转发给了 myDelegate

当把这个 SDK 移除之后，果然就抛了 `doesNotRecognizeSelector:` 的错误

## Runtime 如何实现 weak 属性
[weak 的实现原理](../weak 的实现原理/)


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