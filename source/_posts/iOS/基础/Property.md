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
atomic 性能不佳，可保证 setter/getter 的多线程安全；但是保证不了对集合类的访问的线程安全

### weak 和 assign 的区别
+ weak 用于修饰对象；弱引用不会使引用计数加一，对象释放会自动置为 nil
+ assign 用于修饰基本类型；如果修饰对象，弱引用不会使引用计数加一，但对象释放会变成野指针

### 默认属性是哪些
+ 原子性默认为 atomic，这就是为什么我们一直需要些 nonatomic 的原因
+ 对于 Objective-C 对象，默认为 strong；对于基本类型，默认为 assign

### 常见的修饰
+ NSString/NSArray: copy
+ NSMutableString/NSMutableArray: strong
+ block: copy/strong（如果是 MRC 只能用 copy）


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

见 [如何为协议添加属性](https://norcy.github.io/wiki/iOS/Runtime/如何为协议添加属性)

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
+ 在 category 中定义的所有属性（不再合成实例变量，使用关联对象）
+ 重载的属性，当你在子类中重载了父类中的属性（不再合成实例变量，使用 @synthesize 来手动合成ivar）
