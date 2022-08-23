## 什么是 Block
> Block 是带有自动变量（局部变量）的匿名函数    ——《Objective-C 高级编程》
    
Block 是 Objective-C 对于闭包的实现，本质是一个封装了函数以及函数上下文的对象

+ 可以定义在函数内或函数外
+ 本质是对象

## Block 的写法

### Block 的定义
1. 无参数无返回值

    ```objc
    void (^MyBlockOne)(void) = ^(void){
        NSLog(@"无参数，无返回值");
    };
    MyBlockOne();    // Block的调用
    ```

2. 有参数无返回值

    ```objc
    void (^MyblockTwo)(int a) = ^(int a){
        NSLog(@"@ = %d我就是Block，有参数，无返回值",a);
    };  
    MyblockTwo(100);
    ```

3. 有参数有返回值

    ```objc
    // 声明时可以省略参数的名字
    int (^MyBlockThree)(int,int) = ^(int a,int b){    
        NSLog(@"%d我就是Block，有参数，有返回值",a + b);
        return a + b;
    };  
    int ret = MyBlockThree(12,56);
    ```
    
4. 无参数有返回值(很少用到)

    ```objc
    int (^MyblockFour)(void) = ^{
        NSLog(@"无参数，有返回值");
        return 45;
    };
    int ret = MyblockFour();
    ```


### Block 的省略写法
以上等式的右半部分是 Block 的写法，以下是 Block 的语法，其中表达式就是 Block 的函数内容

```objc
^ 返回值类型 参数列表 表达式
^int(int a, int b) {return a+b;}
```

其中返回值类型可以被省略。省略返回值类型时，如果有 return 语句就使用该返回值的类型，如果有多条 return 语句则它们的类型必须相同，如果没有 return 则为 void

```objc
^ 参数列表 表达式
^(int a, int b) {return a+b;}
```

参数列表也可以被省略，前提是这个 Block 没有参数（而有返回值的时候依然可以省略返回值）

```objc
^ 表达式
^{return @"Hello";}
```

### typedef
实际开发中常用 typedef 定义 Block

```objc
// 最好不要省略参数的名字
typedef int (^MyBlock)(int a, int b);

// 注意这里不是指针
@property (nonatomic, copy) MyBlock myBlock;

// 注意与直接定义 Block 不同，等号右边的返回值是写出来的
self.myBlock = ^int(int a,int b){

}
```

+ 使用 typedef 定义的时候，最好不要省略参数的名字
+ Block 虽然是对象，但是作为属性一般不是指针类型


## 截获外界变量
### 截获自动变量（局部变量）值
    
Block 只捕获在 Block 内部使用的自动变量（局部变量），是值复制而非引用，特别要注意的是默认情况下 Block 只能访问不能修改局部变量的值
    
```objc
int age = 10;
MyBlock block = ^{
    NSLog(@"age = %d", age);
    //age = 11; 会导致编译错误
};
age = 18;
block();    // age = 10
```
    
### 全局变量、静态全局变量和静态局部变量

如果是这些类型，则 Block 直接可以访问到该变量自身，不需要进行拷贝，因此修改生效
    
```objc
int global_val = 10;
static int static_global_val = 10;
int main()
{
    static int static_val = 10;
    myBlock block = ^{
        printf("%d\n", global_val);
        printf("%d\n", static_global_val);
        printf("%d\n", static_val);
    };
    global_val = 18;
    static_global_val = 18;
    static_val = 18;
    block();    // 18,18,18
}
```

### `__block` 修饰的外部变量

对于用 `__block` 修饰的外部变量（称为 `__block 变量`），Block 是复制其引用地址来实现访问的。Block 可以修改 `__block` 修饰的外部变量的值
    
```objc
__block int age = 10;
myBlock block = ^{
    NSLog(@"age = %d", age);
};
age = 18;
block();    // age = 18
```


> 注意： `__block` 变量就是用 `__block` 修饰的局部变量


## Block 的实现
```c
int main()
{
    void (^blk)(void) = ^{printf("Block\n");};

    blk();
}
```

通过 `clang -rewrite-objc MyBlock.c` 可以转化为 C 语言的源码

```c
struct __main_block_impl_0
{
    struct __block_impl impl;
    struct __main_block_desc_0* Desc;
    __main_block_impl_0(void *fp, struct __main_block_desc_0 *desc, int flags=0)
    {
        impl.isa = &_NSConcreteStackBlock;
        impl.Flags = flags;
        impl.FuncPtr = fp;
        Desc = desc;
    }
};

static void __main_block_func_0(struct __main_block_impl_0 *__cself)
{
    printf("Block\n");
}

struct __block_impl
{
    void *isa;
    int Flags;
    int Reserved;
    void *FuncPtr;
};

static struct __main_block_desc_0
{
    size_t reserved;
    size_t Block_size;
} __main_block_desc_0_DATA = { 0, sizeof(struct __main_block_impl_0)};

int main()
{
    // 以下语句等同于
    // void (^blk)(void) = ^{printf("Block\n");};
    struct __main_block_impl_0 tmp = __main_block_impl_0((void *)__main_block_func_0, &__main_block_desc_0_DATA);
    
    struct __main_block_impl_0 *blk = &tmp;
    
    // 以下语句等同于
    // blk()
    void *fun = blk->impl.FuncPtr;
    // void * 转换为函数指针进行调用
    (*((void(*)(__main_block_impl_0 *))(fun)))(blk);
}
```

+ `__main_block_impl_0` 的指针，其命名规则是由函数名（main）和 Block 出现的位置（第 0 个）决定的
+ Block 其实是一个 `__main_block_impl_0` 的指针，其主要包含一个 `__block_impl`，`__block_impl` 主要存储了一个函数指针，指向 Block 的内容
+ Block 的调用就是利用函数指针进行函数调用

### 为什么说 Block 是一个 Objective-C 对象？

```objc
// Objective-C 对象的结构体
struct class_t
{
    struct class_t *isa;
    struct class_t *superclass;
    Cache cache;
    IMP *vtable;
    uintptr_t data_NEVER_USE;
};

// 展开 __block_impl 后，Block 可以表示为以下结构体
struct __main_block_impl_0 
{
    void *isa;
    int Flags;
    int Reserved; 
    void *FuncPtr
    struct __main_block_desc_0* Desc;
};
```
+ `__main_block_impl_0` 相当于基于 `objc_object` 的 Objective-C 类对象的结构体，所以说 Block 是一个 Objective-C 对象
+ 其中，Block 的 isa 的取值可能为 `_NSConcreteGlobalBlock`/`_NSConcreteStackBlock `/`_NSConcreteMallocBlock `

### 截获自动变量（局部变量）的实质
```c
int main()
{
    int val = 5;
    
    void (^blk)(void) = ^{printf("%d\n", val);};

    blk();
}
```

转换后的代码与之前的差别是

```c
struct __main_block_impl_0
{
    struct __block_impl impl;
    struct __main_block_desc_0* Desc;
    int val;
    __main_block_impl_0(void *fp, struct __main_block_desc_0 *desc, int _val, int flags=0):val(_val)
    {
        impl.isa = &_NSConcreteStackBlock;
        impl.Flags = flags;
        impl.FuncPtr = fp;
        Desc = desc;
    }
};

static void __main_block_func_0(struct __main_block_impl_0 *__cself)
{    
    int val = __cself->val;
    printf("%d\n", val);
}
```

最主要的差别就是 `__main_block_impl_0` 自动增加了一个 val 的成员变量，构造函数也发生了相应的变化。注意这里是值复制，这样就可以解释为什么 Block 截获局部变量，在执行 Block 内容时修改其值并不会影响原来的值。同时，因为这种实现无法改变被截获的局部变量的值，所以当在一个 Block 内对一个局部变量进行赋值的时候，编译器就会报错

### 截获全局变量、静态全局变量、静态局部变量的实质
```c
int global_val = 10;
static int static_global_val = 10;
int main()
{
    static int static_val = 10;
    void (^blk)(void) = ^{
        printf("%d\n", global_val);
        printf("%d\n", static_global_val);
        printf("%d\n", static_val);
    };
    global_val = 18;
    static_global_val = 18;
    static_val = 18;
    blk();    // 18,18,18
}
```

转换后的代码与之前的差别是

```c
int global_val = 10;
static int static_global_val = 10;
struct __main_block_impl_0
{
    struct __block_impl impl;
    struct __main_block_desc_0* Desc;
    int *static_val;
    __main_block_impl_0(void *fp, struct __main_block_desc_0 *desc, int *_static_val, int flags=0):static_val(_static_val)
    {
        impl.isa = &_NSConcreteStackBlock;
        impl.Flags = flags;
        impl.FuncPtr = fp;
        Desc = desc;
    }
};

static void __main_block_func_0(struct __main_block_impl_0 *__cself)
{    
    int staic_val = *(__cself->staic_val);
    printf("%d\n", global_val);
    printf("%d\n", static_global_val);
    printf("%d\n", staic_val);
}

int main()
{
    static int static_val = 10;
    struct __main_block_impl_0 tmp = __main_block_impl_0((void *)__main_block_func_0, &__main_block_desc_0_DATA, &static_val);
    
    // 下同，省略
}
```

+ 对于全局变量和静态全局变量，转换后的代码依然可以访问到，因此在 `__main_block_impl_0` 内部并不会新增多余的成员变量

+ 对于静态变量，Block 存储了其地址，从而达到可以修改其值的目的。其实普通局部变量也可以通过传地址的方式来达到在 Block 执行时可以修改其值的目的，但为什么没这么做呢？因为即使保存了普通局部变量的地址，当该变量的作用域失效的时候，那么这个地址也是非法的。而静态局部变量的作用域是一直有效，因此采用存储的地址的方法
    
### 截获 `__block` 变量
```c
int main()
{
    __block int val = 10;

    void (^blk)(void) = ^{printf("%d\n", val);};
    
    val = 1;
    
    blk();    // 1
}
```

转换后的代码与之前的差别是

```c
struct __Block_byref_val_0 
{
    void *__isa;
    __Block_byref_val_0 *__forwarding;  // 实例本身
    int __flags; 
    int __size;
    int val;
};

struct __main_block_impl_0
{
    struct __block_impl impl;
    struct __main_block_desc_0* Desc;
    __Block_byref_val_0 *val;
    __main_block_impl_0(void *fp, struct __main_block_desc_0 *desc, __Block_byref_val_0 *_val, int flags=0):val(_val)
    {
        impl.isa = &_NSConcreteStackBlock;
        impl.Flags = flags;
        impl.FuncPtr = fp;
        Desc = desc;
    }
};

static void __main_block_func_0(struct __main_block_impl_0 *__cself)
{    
    __Block_byref_val_0 *val = __cself->val;
    printf("%d\n", val->_forwording->val);
}

static void __main_block_copy_0(struct __main_block_impl_0 *dst,struct __main_block_impl_0 *src) 
{
  _Block_object_assign(&dst->val, src->val, BLOCK_FIELD_IS_BYREF);
}

static void __main_block_dispose_0(struct __main_block_impl_0 *src) 
{
  _Block_object_dispose(src->val, BLOCK_FIELD_IS_BYREF);
}

static struct __main_block_desc_0 {
  size_t reserved;
  size_t Block_size;
  void (*copy)(struct __main_block_impl_0*, struct __main_block_impl_0*);
  void (*dispose)(struct __main_block_impl_0*);
} __main_block_desc_0_DATA = { 0, sizeof(struct __main_block_impl_0), __main_block_copy_0, __main_block_dispose_0};

int main()
{
    __Block_byref_val_0 val = {0, &val, 0, sizeof(__Block_byref_val_0), 10};

    struct __main_block_impl_0 *blk = &__main_block_impl_0((void *)__main_block_func_0, &__main_block_desc_0_DATA, &val, 0x22000000);
    
    (val.__forwarding->val) = 1;
    
    // 下同，省略
}
```

+ main 函数中，`__block` 变量已经被转化为 `__Block_byref_val_0`，注意转换后的代码没有一个叫做 val 的基本类型的值
+ `__Block_byref_val_0` 最后一个参数表示 `__block` 变量的值，这意味着该结构体持有着与原局部变量值相同的成员变量
+ main 函数中，`__Block_byref_val_0` 的构造函数中，第二个参数传的是它自己（原本是 `__block` 变量）的地址
+ 修改 val 的值是通过 `__forwording` 来实现的，这个例子中 `__forwording` 指向它自己
+ 为什么 `__Block_byref_val_0` 不是在 Block 内部创建呢，而是定义在 Block 的外部？这是为了如果有两个 Block 同时截获同一个局部变量，这两个 Block 需要同时引用这个值，如此才能实现多个 Block 能够修改同一个 `__block` 变量的值
+ 新增了 `__main_block_copy_0` 和 `__main_block_dispose_0` 函数，而这两个函数没有被显式调用，只是作为参数传给了 `__main_block_desc_0` 构造函数，这两个是为了实现正确的引用计数
+ 因为 `__block` 变量被转化为一个带原值的对象，这个对象以指针的形式传到了 Block 内部，因此在 Block 内部修改其值就得以实现

> 注意：`__block` 变量与 Block 的区别：`__block` 变量是栈上的结构体实例，而 Block 是栈上块的结构体实例

### 截获对象
#### `__strong` 类型的对象
```objc
blk_t blk;
{
    id array = [NSMutableArray new];
    blk = [^(id object){
        [array addObject:object];
        NSLog(@"array count = %ld",[array count]);
    } copy];
}
    
blk([NSObject new]);    // array count = 1
blk([NSObject new]);    // array count = 2
blk([NSObject new]);    // array count = 3
```

array 是局部变量，看起来应该会被释放，但是实际上却是被 Block 截获了

转换后的关键代码如下

```c
struct __main_block_impl_0 {
  struct __block_impl impl;
  struct __main_block_desc_0* Desc;
  id __strong array;
  ...
}

static void __main_block_copy_0(...)
{
    _Block_object_assign(...);
}

static void __main_block_dispose_0(...) 
{
    _Block_object_dispose(...);
}
```

+ 截获对象时，Block 内部的成员变量是用 strong 修饰，因此才能使 array 不被释放
+ `__main_block_copy_0` 相当于 retain，会在栈 Block 被复制到堆时被系统调用，使对象的引用计数+1
+ `__main_block_dispose_0` 相当于 release，堆上的 Block 被释放，使对象的引用计数-1
+ 如果以上代码不对 Block 进行 copy，那么虽然 Block 可以捕获 array 并强持有，但是由于还是在栈上，超出其作用域之后，Block 被释放，array 也跟着被释放，后续的 Block 调用会 Crash

#### `__block` + `__strong` 类型的对象
注意，如果是被 `__block` 和 `__strong` 同时修饰的对象，那么区别在于 `__main_block_impl_0` 持有的对象不再是 `id __strong array` 而是 `__Block_byref_obj_0`

```c
struct __Block_byref_obj_0 {
  void *__isa;
__Block_byref_obj_0 *__forwarding;
 int __flags;
 int __size;
 void (*__Block_byref_id_object_copy)(void*, void*);
 void (*__Block_byref_id_object_dispose)(void*);
 __strong id obj;
};
```

除此之外，其结果基本与 `__strong` 类型的对象一致

`__block` 使得对象可以在 Block 内被赋值，否则会编译失败

#### `__weak` 类型的对象
无论是有没有被 `__block` 修饰，`__weak` 类型的对象并不会增加对象的引用计数，所以对象依然会作用域结束时被释放，nil 被赋值给被截获的对象


## Block 的类型
我们先来看看一个由 C/C++/OBJC 编译的程序占用内存分布的结构：

![](https://p1-jj.byteimg.com/tos-cn-i-t2oaga2asx/gold-user-assets/2017/1/19/b5e9425e0b836fcca8d66b398dea079a~tplv-t2oaga2asx-zoom-in-crop-mark:3024:0:0:0.awebp)

block有三种类型：

+ 全局块(`_NSConcreteGlobalBlock`)
+ 栈块(`_NSConcreteStackBlock`)
+ 堆块(`_NSConcreteMallocBlock`)

![](https://p1-jj.byteimg.com/tos-cn-i-t2oaga2asx/gold-user-assets/2017/1/19/84cac3e5509d66965b53755b9a09b441~tplv-t2oaga2asx-zoom-in-crop-mark:3024:0:0:0.awebp)

它们的内存分配如上图

+ 全局块存在于全局内存中，相当于单例
+ 栈块存在于栈内存中，超出其作用域则马上被销毁
+ 堆块存在于堆内存中，是一个带引用计数的对象，需要自行管理其内存

简而言之，存储在栈中的 Block 就是栈块、存储在堆中的就是堆块、既不在栈中也不在堆中的块就是全局块

### `_NSConcreteGlobalBlock`
以下情况均为全局块：

1. 定义在函数之外（即写全局变量的地方）
2. 没有截获任何局部变量（即使定义在函数内部）

### `_NSConcreteStackBlock`
![](https://p1-jj.byteimg.com/tos-cn-i-t2oaga2asx/gold-user-assets/2017/1/19/65f6b6c8a14d8642d021cc3715914099~tplv-t2oaga2asx-zoom-in-crop-mark:3024:0:0:0.awebp)

栈上的 Block，如果其作用域结束，该 Block 就被废弃，如同一般的局部变量。同时，因为 `__block` 变量是被 Block 持有，所以它也会跟着 Block 一起被废弃

### `_NSConcreteMallocBlock`
![](https://p1-jj.byteimg.com/tos-cn-i-t2oaga2asx/gold-user-assets/2017/1/19/ec4b312e57622669e1b354f511792f3d~tplv-t2oaga2asx-zoom-in-crop-mark:3024:0:0:0.awebp)

为了解决栈块在其变量作用域结束之后被废弃（释放）的问题，我们可以把 Block 复制到堆中延长其生命周期

开启 ARC 时，大多数情况下编译器会恰当地进行判断，自动生成将 Block 从栈上复制到堆上的代码

Block 的复制操作执行的是 copy 方法。只要调用了 copy 方法，栈块就会变成堆块

举个例子，编译器会自动 copy 返回的 Block

```c
typedef int (^blk_t)(int);

blk_t func(int rate) {
    return ^(int count) { return rate * count; };
}
```

将 Block 从栈上复制到堆上相当消耗 CPU，所以当 Block 设置在栈上也能够使用时，就不要复制了，因为此时的复制只是在浪费 CPU 资源

![](https://p1-jj.byteimg.com/tos-cn-i-t2oaga2asx/gold-user-assets/2017/1/19/037a2462d7a467a6ebab119237476a9e~tplv-t2oaga2asx-zoom-in-crop-mark:3024:0:0:0.awebp)

栈 Block 复制到堆之后，它的 `__forwarding` 指针会指向堆 Block

通过 `__forwarding`, 无论是在 Block 中还是 Block 外访问 `__block` 变量, 也不管该变量在栈上或堆上, 都能顺利地访问同一个 `__block` 变量

```objc
__block int val = 0;

void (^blk)(void) = [^{++val;} copy];

++val;

blk();
```

其中 ^{++val;} 和 ++val 转换后的代码如下

```objc
// val 是 __block 变量变成的结构体，它含有 __forwarding 指针
++(val.__forwarding->val);
```

### 对各种类型的 Block 执行 copy 操作
![](https://p1-jj.byteimg.com/tos-cn-i-t2oaga2asx/gold-user-assets/2017/1/19/55894ad603f738c74317edde741fb680~tplv-t2oaga2asx-zoom-in-crop-mark:3024:0:0:0.awebp)

### 编译器自动 copy 和不自动 copy 的情况
自动 copy 的情况

1. Block 作为函数返回值返回的时候
2. Cocoa 框架的方法，方法中含有 usingBlock 等（如 NSArray 的 enumerateObjectUsingBlock）
3. GCD 的 API

不自动 copy 的情况：

1. 向函数参数传递 Block

```objc
- (NSArray *)getBlockArray
{
    int val = 10;
    // 不会自动 copy
    return [NSArray arrayWithObjects:^{NSLog(@"%d", val);}, ^{NSLog(@"%d", val);}, nil];
    // 手动 copy
    // return [NSArray arrayWithObjects:[^{NSLog(@"%d", val);} copy],
            [^{NSLog(@"%d", val);} copy], nil];
    // 会自动 copy
    // return @[^{NSLog(@"%d", val);}, ^{NSLog(@"%d", val);}];
}

- (void)main
{
    NSArray *array = [self getBlockArray];
    
    typedef void (^blk_t)(void);
    
    blk_t blk = (blk_t)[array firstObject];
    
    blk();
}
```

+ arrayWithObjects 这个方法返回的数组，将不会包含有效的 Block，执行 blk() 时会 crash
+ 手动 copy 将可以解决 crash 问题
+ 字面量的写法返回的数组，将不会导致 crash（实测如此，待求证）


### 栈 Block 何时会从栈复制到堆
1. 对 Block 调用 copy
2. Block 作为函数返回值返回的时候（编译器自动复制）
3. Cocoa 框架的方法，方法中含有 usingBlock 等（如 NSArray 的 enumerateObjectUsingBlock）（编译器自动复制）
4. GCD 的 API（编译器自动复制）
5. 将 Block 赋值给类的 strong 成员变量

### 多次 copy Block
无论是什么类型的 Block，对 Block 进行多次 copy 都不会有问题。在不确定时调用 copy 方法即可

```objc
blk = [[blk copy] copy];
// 经过多次复制，变量 blk 仍然持有 Block 的强引用，该 Block 不会被废弃
```

改代码等价为

```objc
// 翻译后的代码
{
    blk_t tmp = [blk copy];
    blk = tmp; 
}
{
    blk_t tmp = [blk copy];
    blk = tmp; 
}
```

```objc
// 翻译后的代码+注释
{
    // 初识时，blk 指向一个栈块

    blk_t tmp = [blk copy];
    // 第一次 copy 会将栈块变为堆块，赋值给 tmp，tmp 此时强持有堆块
    
    blk = tmp; 
    // blk 和 tmp 都强持有堆块，blk 原来指向的栈块将会在整个函数结束时被释放
}

// tmp 被释放，此时堆块只被 blk 持有

{
    blk_t tmp = [blk copy];
    // 第二次 copy 会导致堆块的引用计数+1，tmp 强持有堆块
    blk = tmp; 
    // 赋值导致 blk 原有的指向失效，堆块引用计数-1，但此时 tmp 强持有，所以堆块不会被释放
    // blk 再次指向堆块，此时 blk 和 tmp 同时强持有堆块
}

// tmp 被释放，此时堆块只被 blk 持有，引用计数为 1
```

## Block 循环引用
### 使用 Block 成员变量引起的循环引用
```objc
typedeft void (^blk_t)(void);

@interface MyObject : NSObject
{
    blk_t blk_;
}
@end

@implementation MyObject
- (id)init
{
    self = [super init];
    
    blk_ = ^{NSLog(@"self = %@", self);};
    
    return self;
}

- (void)dealloc
{
    NSLog(@"dealloc");
}
@end

int main()
{
    id o = [[MyObject alloc] init];
    
    NSLog(@"%@", o);
    
    return 0;
}

```

以上代码引起循环引用，self 强引用 Block，Block 强引用 self

![](https://upload-images.jianshu.io/upload_images/131615-f329582e615e49b1.png)


Block 截获了类的成员变量时，即使没有使用 self，也会同样截获 self

```objc
@interface MyObject : NSObject
{
    blk_t blk_;
    id obj_;
}
@end

@implementation MyObject
- (id)init
{
    self = [super init];
    
    blk_ = ^{NSLog(@"obj_ = %@", obj_);};
    
    return self;
}
```

对编译器来说，等价于

```objc
blk_ = ^{NSLog(@"obj_ = %@", self->obj_);};
```



破解方法：使用 weak 修饰符

```objc
- (id)init
{
    self = [super init];
    
    id __weak weakSelf = self;
    
    blk_ = ^{NSLog(@"self = %@", weakSelf);};
    
    return self;
}
```

![](https://upload-images.jianshu.io/upload_images/131615-115b9427a9d03692.png)

### 使用 `__block` 变量引起的循环引用
```objc
- (instancetype)init
{
    self = [super init];
    __block id temp = self;//temp持有self
    
    //self持有blk_
    blk_ = ^{
        NSLog(@"self = %@",temp);//blk_持有temp
        temp = nil;
    };
    return self;
}

- (void)execBlc
{
    blk_();
}
```

该源代码没有引起循环引用。但是，如果不调用 execBlock 实例方法（即不执行赋值给成员变量 `blk_` 的 Block），便会循环引用并引起内存泄露


![](https://upload-images.jianshu.io/upload_images/131615-a841db3290ec36b5.png)

破解方法：执行 Block 将 `__block` 变量置空

![](https://upload-images.jianshu.io/upload_images/131615-1098bd25690b25bf.png)

### 两种方法的比较
相比使用 Block 成员变量，使用 `__block` 变量的优点如下：

+ 通过 `__block` 变量可控制对象的持有期间（即不会在执行 Block 之前被释放，不过使用 Block 成员变量可以通过 [Strong-Weak Dance](https://juejin.im/post/5a309c2751882531ba10ed9d) 来解决被截获对象被提早释放的问题）

缺点如下：

+ 为避免循环引用必须执行 Block

## 相关链接
+ [之前写的关于 Block 的笔记](https://norcy.github.io/2016/03/19/Effective-Objective-C-%E8%AF%BB%E4%B9%A6%E7%AC%94%E8%AE%B0/#done-%E7%AC%AC37%E6%9D%A1%EF%BC%9A%E7%90%86%E8%A7%A3%E2%80%9C%E5%9D%97%E2%80%9D%E8%BF%99%E4%B8%80%E6%A6%82%E5%BF%B5)
+ [Strong-Weak Dance](https://juejin.im/post/5a309c2751882531ba10ed9d)
+ [iOS Block 详解](https://juejin.im/entry/588075132f301e00697f18e0)