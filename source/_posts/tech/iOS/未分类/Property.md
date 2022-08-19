## @property 的本质
```objc
@property = ivar(实例变量) + getter/setter（存取方法）;
```

## ARC 下，property 中有哪些属性关键字？

+ 原子性 (atomic, nonatomic)
+ 读写权限 (readonly, readwrite)
+ 内存管理 (strong, weak, unsafe_unretained, retain, assign, copy)
+ 存取方法 (getter, setter)

### atomic 与 nonatomic 区别
+ atomic 性能不佳，可保证 setter/getter 的多线程安全；但是保证不了对集合类的访问的线程安全
+ atomic 保证 setter/getter 安全，也保证不了数据的准确性（线程 A 写，线程 B 写，线程 A 读，读的是线程 B 写入的值）

> [什么是线程安全？](http://lemon2well.top/2018/09/30/iOS%20%E5%BC%80%E5%8F%91/atomic%E6%98%AF%E4%B8%8D%E6%98%AF%E7%BA%BF%E7%A8%8B%E5%AE%89%E5%85%A8%E7%9A%84/)

### weak 和 assign 的区别
+ 都是弱引用，不能使引用计数+1
+ 对象销毁后， weak 修饰的指针会自动变成 nil，而 assign 会成为野指针
+ weak 只能修饰 OC 对象；assign 可以修饰基本数据类型和 OC 对象
+ weak 修饰的成员变量是用 `__weak` 修饰的；assign 生成的成员变量是用 `__unsafe_unretained`

### 默认属性是哪些
+ 基本数据类型默认关键字是 atomic、readwrite、assign 
+ 普通的 Objective-C 对象默认关键字是 atomic、readwrite、strong

### 常见的修饰
+ NSString/NSArray: copy（why）
+ NSMutableString/NSMutableArray: strong（why）
+ block: copy/strong（如果是 MRC 只能用 copy）

> 声明 Block 时，使用 strong 和 retain 会有截然不同的效果。strong 会等于 copy，而 retain 竟然等于 assign

### 关于 copy 与 mutableCopy
+ 非集合类

```objc
NSString *s = @"123";
NSString *s1 = [s copy];          // 浅复制：s1 与 s 是同个对象
NSString *s2 = [s mutableCopy];   // 深复制：s2 是新对象

NSMutableString *m = [@"123" mutableCopy];  
NSMutableString *m1 = [m copy];         // 深复制：m1 是新对象
NSMutableString *m2 = [m mutableCopy];  // 深复制：m2 是新对象
```

结果，s1 是浅复制，没有生成新的对象；其他情况都是深复制，产生了新对象

+ 集合类


```objc
NSArray *a = @[@"123"];
NSString *a1 = [a copy];          // 浅复制：a1 与 a 是同个对象
NSString *a2 = [a mutableCopy];   // 单层深复制：a2 是新对象，但集合里的元素仍然是一样

NSMutableString *b = [@[@"123"] mutableCopy];  
NSMutableString *b1 = [b copy];         // 单层深复制：b1 是新对象，但集合里的元素仍然是一样
NSMutableString *b2 = [mb mutableCopy]; // 单层深复制：b2 是新对象，但集合里的元素仍然是一样
```

结果，a1 是浅复制，没有生成新的对象；其他情况都是 单层深复制，产生了新对象；所谓单层深复制，指的是虽然该指针指向的对象是新产生的，但是集合内部的元素仍然是一样的

更多可以参考：[iOS 集合的深复制与浅复制](https://www.zybuluo.com/MicroCai/note/50592)

## MRC 与 ARC 下的 setter 实现
```objc
// ARC & MRC 的 assign
- (void)setName:(NSString *)name
{
    _name = name;
}

// MRC：retain 的第一种写法
- (void)setName:(NSString *)name
{
    if (_name != name)
    {
        [_name release];
        _name = [name retain];
    }
}

// MRC：retain 的第二种写法
- (void)setName:(NSString *)name
{
    [name retain];  // 先 retain，防止参数是自身
    [_name release];
    _name = name;
}

// MRC：copy
- (void)setName:(NSString *)name
{
    if (_name != name)
    {
        [_name release];
        _name = [name copy];
    }
}

// ARC：copy
- (void)setName:(NSString *)name
{
    if (_name != name)
    {
        _name = [name copy];
    }
}
```

## @synthesize

@synthesize 告诉编译器自动添加 setter/getter，以及添加成员变量

如果不指定实例变量的名字，默认为添加下划线：`@syntheszie var = _var;`

使用场景：协议中声明的属性，实现类需要使用 @synthesize 自动合成

见 [如何为协议添加属性](https://norcy.github.io/wiki/tech/iOS/Runtime/如何为协议添加属性)

## @dynamic
@dynamic 告诉编译器：属性的实例变量、setter 与 getter 方法由用户自己实现，不自动生成；当然对于 readonly 的属性只需提供 getter 即可

如果一个属性被声明为 @dynamic，但是没你有提供 setter/getter，编译不会有问题，运行时会奔溃

而实例变量也需要自己声明

```objc
@interface Person ()
{
    NSInteger _age; // 这个不写会编译报错
}
@property (nonatomic, assign) NSInteger age;
@end

@implementation Person
@dynamic age;
- (void)setAge:(NSInteger)age
{
    _age = age;
}

- (NSInteger)age
{
    return _age;
}
@end
```

## 什么情况不会自动合成
+ 使用了 @dynamic 时（不再合成 setter/getter/实例变量，这 3 个需要全部自己实现）
+ 在 @protocol 中定义的所有属性（不再合成实例变量，使用 @synthesize）
+ 在 category 中定义的所有属性（不再合成实例变量和 getter/setter，使用关联对象）
+ 重载的属性，当你在子类中重载了父类中的属性（不再合成实例变量，使用 @synthesize 来手动合成ivar）
