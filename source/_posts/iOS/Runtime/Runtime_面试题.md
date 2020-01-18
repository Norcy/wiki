## Runtime 怎么添加属性、方法
```objc
class_addIvar           // 添加成员变量
class_addMethod         // 添加方法
class_addProperty       // 添加属性
class_addProtocol       // 添加协议
class_replaceProperty   // 替换属性
```

需要注意的是，成员变量和属性的区别，详见 [方法交换](https://norcy.github.io/wiki/iOS/%E6%96%B9%E6%B3%95%E4%BA%A4%E6%8D%A2/)

## 为什么 Category 中不能动态添加成员变量
因为 `category_t` 的结构中没有含有成员变量的字段，但是含有属性相关的字段，因此虽然无法动态添加成员变量，但是可以动态添加属性

详见：https://norcy.github.io/wiki/iOS/如何为分类添加属性/

## 能否在分类中增加属性
不能添加成员变量；不能直接添加属性，但是可以通过 Runtime 的方法添加

因为方法和属性并不“属于”类实例，而成员变量“属于”类实例

因为 Category 在运行期，对象的内存布局已经确定，如果添加实例变量就会破坏类的内部布局。

详见：https://norcy.github.io/wiki/iOS/如何为分类添加属性/

## 能否向编译后得到的类中增加实例变量？能否向运行时创建的类中添加实例变量？
不能向编译后得到的类中增加实例变量；能向运行时创建的类中添加实例变量；

添加实例变量会影响类结构体中的 `objc_ivar_list` 和 `instance_size`

因为编译后的类已经注册在 Runtime 中，类结构体中的 objc_ivar_list 实例变量的链表和 instance_size 实例变量的内存大小已经确定，同时 Runtime 会调用 class_setIvarLayout 或 class_setWeakIvarLayout 来处理 strong weak 引用，所以不能向存在的类中添加实例变量

运行时创建的类是可以添加实例变量，调用 class_addIvar 函数，但是得在调用 objc_allocateClassPair 之后，objc_registerClassPair 之前，原因同上。 

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

## `_objc_msgForward` 是什么
`_objc_msgForward` 是 IMP 类型，当向一个对象发送一条消息，会调用 methodForSelector 方法，但它并没有实现的时候，该方法会返回该 IMP，即 `_objc_msgForward`，这个 IMP 用于执行消息转发




