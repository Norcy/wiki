+ [Runtime 脑图](http://naotu.baidu.com/file/30a56f1e1bbb7af7f876353dda6f6b2c)

# 概念

[Class、id、`objc_object` 定义源码](https://opensource.apple.com/source/objc4/objc4-750/runtime/objc.h.auto.html)

[`objc_class`、`objc_method` 定义源码](https://opensource.apple.com/source/objc4/objc4-750/runtime/runtime.h.auto.html)

[`category_t` 定义源码](https://opensource.apple.com/source/objc4/objc4-750/runtime/objc-runtime-new.h.auto.html)

## 实例对象（objc_object）
```objc
typedef struct objc_object *id;
```

id 是一个指向 `objc_object` 的指针

```objc
struct objc_object {
    isa_t isa;
}

union isa_t 
{
	Class cls;
	// ... 省略其他
}
```

简单点说，`objc_object` 中包含了 Class 信息

## 类对象（objc_class）

```objc
typedef struct objc_class *Class;
```

Class 是一个指向 `objc_class` 的指针

在 Objc2.0 之前，`objc_class` 源码如下

```objc
struct objc_class
{
	Class isa OBJC_ISA_AVAILABILITY;

#if !__OBJC2__
	Class super_class 	OBJC2_UNAVAILABLE;
	const char *name 	OBJC2_UNAVAILABLE;
	long version 		OBJC2_UNAVAILABLE;
	long info 			OBJC2_UNAVAILABLE;
	long instance_size 	OBJC2_UNAVAILABLE;
	struct objc_ivar_list *ivars 			OBJC2_UNAVAILABLE;
	struct objc_method_list **methodLists 	OBJC2_UNAVAILABLE;
	struct objc_cache *cache 				OBJC2_UNAVAILABLE;
	struct objc_protocol_list *protocols 	OBJC2_UNAVAILABLE;
#endif

} OBJC2_UNAVAILABLE;
```

然后在 2006 年苹果发布 Objc2.0 之后，`objc_class` 的定义就变成下面这个样子了

```objc
struct objc_class : objc_object {
    Class superclass;
    cache_t cache;             
    class_data_bits_t bits;
}
```

在 Objc2.0 中，所有的对象都会包含一个`isa_t` 类型的 isa 指针

`objc_class` 继承于 `objc_object`，所以 `objc_class` 也包含了 isa 指针，故类本身其实也是一个对象，称之为类对象

这意味着，可以像向实例对象发送消息一样，我们可以向类对象发送消息，比如 `[Person alloc]`

`class_data_bits_t` 存储着类对象的相关数据（比如对象方法、对象成员等等），Objc2.0 之前存储在 `objc_class` 的信息都放到这里了


当一个对象的实例方法被调用的时候，会通过 isa 找到相应的类，然后在该类的`class_data_bits_t` 中去查找方法实现

但是在我们调用类方法的时候，类对象的 isa 里面是什么呢？这里为了和对象查找方法的机制一致，遂引入了元类的概念

## 元类

![](https://user-gold-cdn.xitu.io/2018/4/1/1628088a3e4f0167)

<!--![](http://yulingtianxia.com/resources/Runtime/class-diagram.jpg)-->

上图可以看出：

1. Superclass
	
	+ 实例对象之间没有父子关系，有父子关系的是类对象和元类
	+ NSObject 类的父类是 nil
	+ 特殊：NSObject 元类的父类是 NSObject 类对象

2. isa
	
	可以理解“是一个”

	+ 实例对象“是一个”类对象，类对象“是一个”元类
	+ 特殊：任何元类都“是一个” NSObject 元类

+ 【消息发送】：当一个消息发送给任何一个对象时，将会由对象的 isa 指针开始查找方法，接着沿着 superclass 链向上去查找
+ 【内存布局】：不同实例对象对应的类，父类，元类打印出来的地址相同，因为 main 方法执行之前，类对象和元类对象就被创建

### 理解好对象、类和元类的关系（为什么需要元类）
```objc
Person *p = [[Person alloc] init];
```

其中，p 是对象，Person 是类。注意，**类本身也是对象**，所以我们可以把类叫做类对象

对象的实例方法存放在类对象中，对象是以类对象为模版进行创建

而**类对象与元类的关系好比对象和类对象的关系**，元类中保存了创建类对象以及类方法所需的所有信息

+ 当你给对象发送消息时，消息是从这个对象的类的方法列表中查找
+ 当你给类发消息时，消息是从这个类的元类的方法列表中查找

到此，我们可以理解，为什么类对象“是一个”元类；也可以理解为什么需要元类这个概念（为了和对象查找方法的机制一致）

类对象和元类对象是唯一的，对象是可以在运行时创建无数个的。而在 main 方法执行之前，类对象和元类对象被创建

### 引申思考：究竟什么样的数据结构才算对象？
每个对象都有一个类，这个是 Objective-C 中关于面向对象的内容

实际上，任何数据结构，只要其内存区域的第一块位置是以指向 Class 的指针都可以被称之为对象

比如 `objc_object`，Objc2.0 前的 `objc_class`， Objc2.0 后的 `objc_class` 

```objc
// isa_t 展开之后
struct objc_object {
    Class isa;
}

// Objc2.0 前
struct objc_class
{
	Class isa;
	...
}

// Objc2.0 后，继承关系展开之后
struct objc_class : objc_object {
    Class isa;
    Class superclass;
    ...
}
```

### 引申思考：类对象是一个对象，那么元类是对象吗？
类对象是 `objc_class`，它的 isa 指针指向的是一个 `objc_class`，这就意味着元类是一个 `objc_class` 描述的结构。由于 `objc_class` 的第一个位置存放着 Class 指针，所以元类本身也是一个对象！所有的元类的 Class 指针指向的都是根元类

即所有元类的类都是根元类，这个是规则。由于这条规则，根元类（即 NSObject 类的元类）的 isa 指针指向了自己，也就是说根元类是它自己的一个实例

### 引申思考：`objc_object` 对应对象，`objc_class` 对应类对象，那是不是会有 `objc_meta_class` 对应元类
id 是一个指向 `objc_object` 的指针，Class 是一个指向 `objc_class` 的指针，由此看，id 对应对象，Class 对应类对象确实没错，id 和 Class 都是开发者层面能接触到的，但是元类开发者并不会接触到，所以没有一个对应的“开发层面的概念”对应元类。

元类本身也是一个对象，是用 `objc_class` 来描述。

即对象用 `objc_object` 描述；类对象和元类都是用 `objc_class` 描述

### 引申思考：为什么所有元类的 isa 指针直接指向了根元类？

这是规定（我猜：指向 nil 或其他地方都没有指向根元类合理），同时元类的 isa 指针不是很重要，因为在现实世界中没人向元类发送消息

### 引申思考：为什么 NSObject 元类的父类是 NSObject 类对象？
因为 NSObject 定义了一系列“抽象方法”，这里面既有实例方法，又有类方法

+ - class
+ + class
+ - isMemberOfClass
+ - isKindOfClass
+ - respondsToSelector
+ - conformsToProtocol
+ + conformsToProtocol
+ - methodForSelector
+ + instanceMethodForSelector

注意：这些方法中的类方法列表是存放在 NSObject 根元类的数据结构中，但是这些方法是“写”在 NSObject.m 中

定义 NSObject 元类的父类为 NSObject 类对象，导致了所有的实例对象、类对象和元类对象都是 NSObject 类对象的实例（除了 NSObject 类对象自己）

> NSObject 类对象是万物之根

意义在于，元类对象也能调用 NSObject 的实例方法?

如果一个类的类方法没有被实现，最终会去 NSObject 的实例方法中寻找

+ 对于所有的实例对象，都能够调用到 NSObject 的实例方法
+ 对于所有的类对象和元类对象，都能够调用到 NSObject 的类方法

> For all instances, classes and meta-classes in the NSObject hierarchy, this means that all NSObject instance methods are valid. For the classes and meta-classes, all NSObject class methods are also valid. ——引自[《What is a meta-class in Objective-C?》](http://www.cocoawithlove.com/2010/01/what-is-meta-class-in-objective-c.html)


## Method（objc_method）
```objc
struct objc_method {
    SEL method_name;		// 方法名（区分方法的标识）
    char *method_types;		// 返回值类型
    IMP method_imp;			// 方法实现
}
```

SEL 与 IMP 的关系类似于 key 与 value 的关系

## SEL（objc_selector）
```objc
typedef struct objc_selector *SEL;

struct objc_selector {
    char *name;			// 方法名称
    char *types;		// 返回值类型
};
```

SEL 是一个指向 `objc_selector` 的指针，可以理解为方法（Method）的 ID

IMP 可以理解为函数指针，指向了最终的实现

### 引申思考：关于重载

Objective-C 中不支持函数重载就是因为 SEL 只记录了方法名而没有参数，同时一个类的方法列表中不能存在两个相同的 SEL 

### 引申思考：关于重写

不同的类可以有同一个 SEL，这些 SEL 对应着不同的实现

不同类的实例对象执行相同的 SEL 时，会在各自的方法列表中去根据 SEL 去寻找自己对应的 IMP

这使得 OC 可以支持函数重写

比如父类和子类都有 viewDidLoad，父类和子类是两个不同的类对象，同时有相同的 SEL，即 viewDidLoad，而其 IMP 又不一样

```objc
@property SEL selector;
```

Objective-C 中的 selector 是 SEL 的一个实例对象

我们可以用 `@selector()` 返回的是一个 SEL 类型的方法选择器


## 方法实现（IMP）
```objc
typedef id (*IMP)(id, SEL, ...); 
```

**关于函数指针**

先了解函数指针的概念

> typedef 返回类型(`*`函数指针名)(参数表)


```c
typedef void (*funPointer)(int); 
funPointer pFun;
void myFun(int a){print("Hello");} 
void main() 
{ 
    pFun = myFun; 
    (*pFun)(2); 
}
```

所以 IMP 是一个函数指针，指向类似 `id FunName(id, SEL, ...){...}` 的函数实现

在 Runtime 中，IMP 指向着方法最终实现的内存地址

## 类缓存（objc_cache）
Runtime 中，每当找到一个类的方法时，会放入它的缓存，即 `objc_class` 中的 `objc_cache`，以加快下次方法调用时的查找效率


## 分类（objc_category）
```objc
struct category_t { 
    const char *name; 							// 原类名，而不是分类名
    // 要扩展的类对象，编译期间是不会定义的，而是在 Runtime 阶段通过 name 对应到对应的类对象
    classref_t cls; 							
    struct method_list_t *instanceMethods; 		// 分类中新增的对象方法列表
    struct method_list_t *classMethods;			// 分类中新增的类方法列表
    struct protocol_list_t *protocols;			// 分类中新增的协议列表
    struct property_list_t *instanceProperties;	// 分类中新增的属性列表
};
```

### 引申思考：[如何在分类中添加属性](../如何为分类添加属性)
可以看出，分类中可以添加实例方法，类方法，甚至可以实现协议，添加属性，但不可以添加成员变量

instanceProperties 的存在是我们可以通过 `objc_setAssociatedObject` 和 `objc_getAssociatedObject` 向分类中增加实例变量的原因，不过这个和一般的实例变量是不一样的


# 消息发送（`objc_msgSend`）
一个对象的方法像这样[obj foo]，编译器转成 `objc_msgSend(obj, foo)`

```objc
id objc_msgSend(id self, SEL op, ...)
```

Runtime 时执行的流程是这样的：

1. 通过 obj 的 isa 指针找到它的类对象（obj 是一个 id，即 `objc_object` 指针，有 isa 指针）
2. 在类对象的方法列表中寻找 foo（`objc_class` 的 methodLists）
3. 如果类对象中没到 foo，继续往它的 superclass 中找
4. 一旦找到 foo 这个函数（`objc_method`），就去执行它的实现 IMP，并转发 IMP 的返回值	

如果每次消息传递都沿着继承链，在每个类的 methodLists 查找方法其实效率很低，所以需要缓存，即 `objc_class` 中的 `objc_cache`，key 是 `objc_method` 中的SEL，value 是 `objc_method` 中的IMP



# 消息转发（`_objc_msgforward`）
如果消息传递的过程中，沿着继承树查找到最终的根类（NSObject）还是没有对应的方法实现，则会进行消息转发，如果消息转发失败了就回执行 `doesNotRecognizeSelector:` 方法报`unrecognized selector` 错

什么是消息转发呢，主要分为以下三个阶段

![](https://user-gold-cdn.xitu.io/2018/4/1/1628088a3e48a485)

## 动态方法解析
第一个阶段 Runtime 会调用 `+resolveInstanceMethod:` 或者 `+resolveClassMethod:`（取决于是实例方法还是类方法），让你有机会提供一个函数实现。如果你添加了函数并返回YES， 那运行时系统就会重新启动一次消息发送的过程

```objc
- (void)viewDidLoad 
{
    [super viewDidLoad];
    [self performSelector:@selector(foo:)];
}

+ (BOOL)resolveInstanceMethod:(SEL)sel 
{
	if (sel == @selector(foo:)) 
	{
		// 注意参数 fooMethod 这里是作为一个函数指针传递
		class_addMethod([self class], sel, (IMP)fooMethod, "v@:");
		return YES;
    }
    return [super resolveInstanceMethod:sel];
}

void fooMethod(id obj, SEL _cmd) 
{
    NSLog(@"Hello");
}
```

## 快速转发（替换消息接收者）
如果你错过了第一阶段，则进入第二阶段。Runtime 会调用 forwardingTargetForSelector 给你把这个消息转发给其他对象的机会

```objc
- (id)forwardingTargetForSelector:(SEL)aSelector 
{
	if (aSelector == @selector(foo)) 
	{
		return [Person new];
	}
	 
	return [super forwardingTargetForSelector:aSelector];
}
```

## 完整消息转发
最后一步会发送 `-methodSignatureForSelector:` 消息获得函数的参数和返回值类型。

+ 如果返回nil ，Runtime 则会发出 `-doesNotRecognizeSelector:` 消息，程序这时也就挂掉了
+ 如果返回了一个函数签名，Runtime 就会创建一个 NSInvocation 对象并发送 `-forwardInvocation:` 消息给目标对象

```objc
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector 
{
	if ([NSStringFromSelector(aSelector) isEqualToString:@"foo"]) 
	{
		// 返回签名，进入 forwardInvocation
		return [NSMethodSignature signatureWithObjCTypes:"v@:"];
	}
	return [super methodSignatureForSelector:aSelector];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation 
{
	SEL sel = anInvocation.selector;
	
	Person *p = [Person new];
	if ([p respondsToSelector:sel]) 
	{
		[anInvocation invokeWithTarget:p];
	}
	else 
	{
		[self doesNotRecognizeSelector:sel];
	}
}
```

什么是 `"v@:"`，详细可参见[Type Encodings](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtTypeEncodings.html#//apple_ref/doc/uid/TP40008048-CH100-SW1)

所以不同与第二阶段，在这个阶段你可以：

+ 把消息存储，在你觉得合适的时机转发出去，或者不处理这个消息。
+ 修改消息的 target，selector，参数等
+ 多次转发这个消息，转发给多个对象

显然在这个阶段，你可以对消息做更多的事情，是第二个阶段的扩充


## 引申思考：消息转发为什么要三个阶段呢？
第一阶段意义在于动态添加方法实现，第二阶段直接把消息转发给其他对象，第三阶段是对第二阶段的扩充，可以实现多次转发，转发给多个对象等

如果只是考虑消息转发的功能，那么只提供最后一个阶段就可以实现，所以这个问题的本质是，为什么要多提供前面两个阶段？

Objective-C 的消息机制与 C++ 等静态编译语言不同，提供动态性的同时必然也牺牲了调用的效率。消息转发的效率必然不如 C++ 的函数调用。

从效率上讲，完整的消息转发效率太低，提供前面两个阶段就是为了让消息能够尽快得到处理；因为三个阶段步骤越往后，处理消息累计开销就越大。

[《Objective-C 与 Runtime：为什么是这样？》（R.I.P）](http://www.cocoachina.com/articles/13336)

# Runtime 应用
## [Swizzle Method](../%E6%96%B9%E6%B3%95%E4%BA%A4%E6%8D%A2/)
## [自定义 KVO 实现](http://liuduo.me/2018/02/07/kvo-imp/)
## [给分类添加属性](../%E5%A6%82%E4%BD%95%E4%B8%BA%E5%88%86%E7%B1%BB%E6%B7%BB%E5%8A%A0%E5%B1%9E%E6%80%A7/)
## 打印类的所有属性值
```objc
- (NSString *)description
{
    return [NSString stringWithFormat:@"%@", [self properties_values]];
}

- (NSDictionary *)properties_values
{
    NSMutableDictionary *props = [NSMutableDictionary dictionary];
    unsigned int outCount, i;
    objc_property_t *properties = class_copyPropertyList([self class], &outCount);
    for (i = 0; i < outCount; i++)
    {
        objc_property_t property = properties[i];
        const char *char_f = property_getName(property);
        NSString *propertyName = [NSString stringWithUTF8String:char_f];
        id propertyValue = [self valueForKey:propertyName];
        if (propertyValue)
        {
            [props setObject:propertyValue forKey:propertyName];
        }
    }
    free(properties);
    return props;
}
```

可以将 `properties_values` 方法，写到 NSObject 的分类里，就不需要每个人都写一份

## 实现 NSCoding 的自动归档和自动解档
OC 中归档解档又称为序列化和反序列化

归档/解档需要实现 NSCoding 协议方法，在 NSCoding 协议方法中实现了对每个属性分别进行归档/解档，归档对属性值归档为相应的字段，解档依据相应的字段为对象属性赋值

可以使用 Runtime 的 `Ivar *class_copyIvarList` 方法获取某个类的属性个数和属性列表。遍历属性列表，可获取每个属性的名字，然后使用 __KVC__ 获取或设置每个属性的值

```objc
- (void)encodeWithCoder:(NSCoder *)coder
{
    [self code:YES coder:coder];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if (self = [super init]) 
    {
        [self code:NO coder:coder];
    }
    return self;
}

- (void)code:(BOOL)isEncode coder:(NSCoder *)coder
{
    unsigned int outCount = 0;
    Ivar *ivars = class_copyIvarList([UserModel class], &outCount);
    
    for (int i = 0; i < outCount; i++) 
    {
        Ivar ivar = ivars[i];
        const char * name = ivar_getName(ivar);
        NSString *key = [NSString stringWithUTF8String:name];
        id value;
        if (isEncode) 
        {
            value = [self valueForKey:key];
            [coder encodeObject:value forKey:key];
        }
        else 
        {
            value = [coder decodeObjectForKey:key];
            [self setValue:value forKey:key];
        }
    }
    free(ivars);
}
```

## 实现字典和模型的自动转换（MJExtension）
用 Runtime 提供的函数遍历 Model 自身所有属性，如果属性在 json 中有对应的值，则将其赋值

核心方法：在 NSObject 的分类中添加方法

```objc
- (instancetype)initWithDict:(NSDictionary *)dict 
{
    if (self = [self init]) 
    {
        NSMutableArray *keys = [NSMutableArray array];
        NSMutableArray *attributes = [NSMutableArray array];
        unsigned int outCount;
        objc_property_t *properties = class_copyPropertyList([self class], &outCount);
        for (int i = 0; i < outCount; i ++) 
        {
            objc_property_t property = properties[i];
            NSString * propertyName = [NSString stringWithCString:property_getName(property) encoding:NSUTF8StringEncoding];
            [keys addObject:propertyName];
            
            NSString * propertyAttribute = [NSString stringWithCString:property_getAttributes(property) encoding:NSUTF8StringEncoding];
            [attributes addObject:propertyAttribute];
        }

        free(properties);

        for (NSString * key in keys) 
        {
            if ([dict valueForKey:key] == nil) continue;
            [self setValue:[dict valueForKey:key] forKey:key];
        }
    }
    return self;
}
```

## 热更新（JSPatch）
JS 传递字符串给 OC，OC 通过 Runtime 接口调用和替换 OC 方法

# 参考文章
+ [iOS Runtime详解](https://juejin.im/post/5ac0a6116fb9a028de44d717#heading-1)
+ [神经病院 Objective-C Runtime 入院第一天—— isa 和 Class](https://halfrost.com/objc_runtime_isa_class/)
+ [《What is a meta-class in Objective-C?》](http://www.cocoawithlove.com/2010/01/what-is-meta-class-in-objective-c.html)