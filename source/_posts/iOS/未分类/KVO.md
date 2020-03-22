## 基本概念
+ KVO 是 Key-Value Observing 的缩写，用于监听对象的某一属性改变
+ KVO 是 Objective-C 对观察者模式的实现

## 基础用法
[我的博客园](https://www.cnblogs.com/chenyg32/p/4808246.html)


## 实现原理
KVO 是使用 isa 混写技术（isa-swizzing）实现

当一个对象使用了 KVO 监听，iOS 系统会修改这个对象的 isa 指针，使其指向通过 Runtime 动态创建的子类，该子类重写了 set 方法，内部实现会调用 willChangeValueForKey、父类的 setter、didChangeValueForKey。在 didChangeValueForKey 方法中又会调用监听器的监听方法

原理如图所示

![](https://ask.qcloudimg.com/http-save/yehe-2149899/94pqoq2lrt.png)
![](https://ask.qcloudimg.com/http-save/yehe-2149899/dl4xa5lq3k.png)

重点如下：

1. 动态创建了子类（修改了 isa 指针的指向）
    
    > RuntimeAPI : objc_allocateClassPair 和 objc_registerClassPair，动态生成 NSKVONotifying_XXX

2. 重写了 setter 方法

    1. 调用 willChangeValueForKey
    2. 调用 super 的 setter
    3. 调用 didChangeValueForKey
    4. didChangeValueForKey 中调用 observeValueForKeyPath:ofObject:change:context:

## 相关面试题
### 1. 什么是 isa 混写 / KVO 的实现原理
见上节

### 2. 如何手动触发 KVO
手动调用 willChangeValueForKey 和 didChangeValueForKey，缺一不可

```objc
Person *p = [[Person alloc] init];
[p addObserver:self forKeyPath:@"age" options:NSKeyValueObservingOptionNew context:nil];
[p willChangeValueForKey:@"age"];
[p didChangeValueForKey:@"age"];
```

### 3. 直接修改成员变量会触发 KVO 吗
比如 `_age = 20` 这样的调用不会触发 KVO，因为没有调用 setter 方法

### 4. KVC 会触发 KVO 吗
KVC 访问属性既可以通过 setter，也可能通过直接访问成员变量（`+ (BOOL)accessInstanceVariablesDirectly` 返回 NO 的时候就不会触访问成员变量）；会不会触发 KVO 本质上是取决于会不会触发 setter 函数


## 细节探究
### 创建的子类的伪代码实现
```objc
@implementation NSKVONotifying_Person
- (Class)class
{
    return [Person class];  // 而不是 return object_getClassName(self);
}

- (void)setAge:(NSInteger)age
{
    _NSSetLongLongValueAndNotify();
}

void _NSSetLongLongValueAndNotify()
{
    [self willChangeValueForKey:@"age"];
    [super setAge:age];
    [self didChangeValueForKey:@"age"];
}

- (void)didChangeValueForKey:(NSString *)key
{
    [oberser observeValueForKeyPath:key ofObject:self change:nil context:nil];
}
@end
```

### 动态创建子类以及重写 class 方法的代码验证

```objc
Person *p = [[Person alloc] init];
NSLog(@"添加监听之前 %@, %@", object_getClass(p), [p class]);
[p addObserver:self forKeyPath:@"name" options:NSKeyValueObservingOptionNew context:nil];
NSLog(@"添加监听之后 %@, %@", object_getClass(p), [p class]);
```

输出

```sh
添加监听之前 Person, Person
添加监听之后 NSKVONotifying_Person, Person
```

1. 可以看到 p 的 isa 指针已经被改变了，指向了一个 `NSKVONotifying_` 开头的类，该类是动态生成的。
2. 尽管如此，p 的 class 函数还是返回了用户的类，实际上是苹果重写了该方法，目的是不想让这个内部细节暴露给开发者；不重写的话，使用 `[person class]` 就会返回 `NSKVONotifying_Person`，这是苹果所不希望看到的，注意，class 方法的默认实现如下

```objc
- (Class)class
{
    return object_getClass(self);   // 将会返回 isa 指针的实际指向，即 NSKVONotifying_Person
}
```

### 重写子类 set 方法的代码验证

```objc
Person *p = [[Person alloc] init];
NSLog(@"添加监听之前的方法地址：%p", [p methodForSelector:@selector(setAge:)]);
[p addObserver:self forKeyPath:@"age" options:NSKeyValueObservingOptionNew context:nil];
NSLog(@"添加监听之后的方法地址：%p", [p methodForSelector:@selector(setAge:)]);
```

输出如下，可以使用 p 命令将地址强制转为函数

```sh
添加监听之前的方法地址：0x10c9f0bc0
添加监听之后的方法地址：0x7fff257228bc

(lldb) p (IMP)0x10c9f0bc0
(IMP) $0 = 0x000000010c9f0bc0 (ForTest`-[Person setAge:] at Person.h:14)
(lldb) p (IMP)0x7fff257228bc
(IMP) $1 = 0x00007fff257228bc (Foundation`_NSSetLongLongValueAndNotify)
```

因为 age 是 NSInteger，所以调用了 `_NSSetLongLongValueAndNotify`，如果是其他数据类型就会调用对应的方法

使用以下命令可以查看 Foundation 中包含 ValueAndNotify 的方法

```sh
nm -a /System/Library/Frameworks/Foundation.framework/Versions/C/Foundation | grep ValueAndNotify
```

输出

```sh
000000000015ca93 t __NSSetBoolValueAndNotify
00000000000553ab t __NSSetCharValueAndNotify
00000000000c6bfb t __NSSetDoubleValueAndNotify
0000000000101f6a t __NSSetFloatValueAndNotify
00000000001321e9 t __NSSetIntValueAndNotify
0000000000052c7b t __NSSetLongLongValueAndNotify
00000000001c02c7 t __NSSetLongValueAndNotify
0000000000070df9 t __NSSetObjectValueAndNotify
00000000000cc7f1 t __NSSetPointValueAndNotify
00000000001c07be t __NSSetRangeValueAndNotify
0000000000092242 t __NSSetRectValueAndNotify
00000000001c053f t __NSSetShortValueAndNotify
00000000000cc624 t __NSSetSizeValueAndNotify
00000000001c0183 t __NSSetUnsignedCharValueAndNotify
00000000000d4d60 t __NSSetUnsignedIntValueAndNotify
000000000008cd85 t __NSSetUnsignedLongLongValueAndNotify
00000000001c0401 t __NSSetUnsignedLongValueAndNotify
00000000001c0678 t __NSSetUnsignedShortValueAndNotify
00000000001bf600 t __NSSetValueAndNotifyForKeyInIvar
00000000001bf662 t __NSSetValueAndNotifyForUndefinedKey
00000000001c091e t ____NSSetBoolValueAndNotify_block_invoke
00000000000554eb t ____NSSetCharValueAndNotify_block_invoke
00000000000c6d45 t ____NSSetDoubleValueAndNotify_block_invoke
00000000001020b3 t ____NSSetFloatValueAndNotify_block_invoke
0000000000136a41 t ____NSSetIntValueAndNotify_block_invoke
0000000000052e23 t ____NSSetLongLongValueAndNotify_block_invoke
00000000001c0989 t ____NSSetLongValueAndNotify_block_invoke
0000000000091811 t ____NSSetObjectValueAndNotify_block_invoke
0000000000125637 t ____NSSetPointValueAndNotify_block_invoke
00000000001c0a4a t ____NSSetRangeValueAndNotify_block_invoke
00000000000923db t ____NSSetRectValueAndNotify_block_invoke
00000000001c09ed t ____NSSetShortValueAndNotify_block_invoke
0000000000122c1a t ____NSSetSizeValueAndNotify_block_invoke
00000000001c0958 t ____NSSetUnsignedCharValueAndNotify_block_invoke
00000000000d4e9d t ____NSSetUnsignedIntValueAndNotify_block_invoke
00000000000ab280 t ____NSSetUnsignedLongLongValueAndNotify_block_invoke
00000000001c09b6 t ____NSSetUnsignedLongValueAndNotify_block_invoke
00000000001c0a1a t ____NSSetUnsignedShortValueAndNotify_block_invoke
```

## 参考资料
[面试驱动技术 - KVO && KVC](https://cloud.tencent.com/developer/article/1403599)