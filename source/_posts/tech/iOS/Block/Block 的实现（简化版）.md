## 不捕获任何变量
```c
int main()
{
    void (^blk)(void) = ^{printf("Block\n");};
    blk();
}
```

```c
// block 转化后的类
struct block_struct
{
	// 构造函数传入函数实现
    block_struct(void *fp)
    {
        this.FuncPtr = fp;	// 函数实现
    }
};

// block 的内容被转化为 1 个全局函数
static void block_imp(block_struct *self)
{
    printf("Block\n");
}

int main()
{
    // 以下语句等同于 void (^blk)(void) = ^{printf("Block\n");};
    block_struct blk(block_imp);
    // 以下语句等同于 blk()
    blk.FuncPtr(blk);
}
```

1. Block 的内容被转化为 1 个全局函数
2. Block 本身被转为一个 struct，其构造函数传入这个全局函数的地址
3. Block 的调用等同于，调用 struct 的函数实现，参数是自身

## 捕获局部变量
```c
int main()
{
    int val = 5;
    void (^blk)(void) = ^{printf("%d\n", val);};
    blk();
}
```

```c
struct block_struct
{
	int val;	// 多了变量
    block_struct(void *fp, int _val):val(_val)	// 构造函数多了个参数
    {
        this.FuncPtr = fp;
    }
};

static void block_imp(block_struct *self)
{
    printf("%d\n", self->val);
}

int main()
{
	int val = 5;
    block_struct blk(block_imp, val);
    blk.FuncPtr(blk);
}
```

1. 不同之处是转换后的结构体多了一个 int 的成员变量，构造函数和函数实现也相应的变化了
2. 因为是值复制，所以无法在 Block 对捕获的简单类型值进行赋值

## 捕获全局变量、静态全局变量、静态局部变量
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

```c
int global_val = 10;
static int static_global_val = 10;
struct block_struct
{
	int *static_val;	// 指针
    block_struct(void *fp, int _static_val):static_val(_static_val)
    {
        this.FuncPtr = fp;
    }
};

static void block_imp(block_struct *self)
{
    printf("%d\n", global_val);
    printf("%d\n", static_global_val);
    printf("%d\n", *(self->staic_val));
}

int main()
{
    static int static_val = 10;
    block_struct blk(block_imp, &static_val);
    global_val = 18;
    static_global_val = 18;
    static_val = 18;
    blk.FuncPtr(blk);
}
```

1. 对于全局变量，`block_struct` 没有增加变量；对于局部静态变量，存储的是指针
2. 这样就可以在 Block 内部任意的修改这些值

## 捕获 `__block` 变量
```c
int main()
{
    __block int val = 10;
    void (^blk)(void) = ^{printf("%d\n", val);};
    val = 1;
    blk();    // 1
}
```

```c
struct block_struct
{
    block_val *val;    // 多了变量
    block_struct(void *fp, int _val):val(_val)  // 构造函数多了个参数
    {
        this.FuncPtr = fp;
    }
};

struct block_val
{
    block_val *forwarding;
    int val;
};

static void block_imp(block_struct *self)
{
    printf("%d\n", self->val->forwarding->val);
}

int main()
{
    block_val val(&val, 10);
    block_struct blk(block_imp, val);
    (val.forwarding->val) = 1;
    blk.FuncPtr(blk);   // 1
}
```

1. `__block` 变量被转为一个类
2. 无论是 Block 内还是外，访问值都得通过 forwarding 指针
3. Block 得到的是一个对象的指针，因此读写方面可以和 Block 外部的值保持一致

这个例子中 forwarding 指针都是自己，不能体现其意义，那么 forwarding 指针有什么用呢？

我们知道，当对一个栈 Block 进行 copy 时，copy 函数会返回一个堆 Block。

copy 调用的时候，会将此堆 Block 访问到的 `block_val` 的 forwarding 指针，指向堆 Block 新生成的 `block_val`，而堆 Block 自己的 `block_val` 的 forwarding 指针会指向自己

这样原本在栈上的 `block_val`，或者是堆 Block 的 `block_val`，访问到的对象都会是同一个

```objc
__block int val = 0;
void (^blk)(void) = [^{++val;} copy];
++val;  //1
blk();  //2
```

```objc
block_val val(&val, 0); // 这是一个在栈上的 block_val
block_struct temp(block_imp, val);  // 这是一个在栈上的 Block
block_struct blk = _Block_copy(temp);  // 这是一个在堆上的 Block，会新生成一个新的 block_val，同时这个过程会修改 block_val 的 forwarding 指向这个新的 block_val
(val.forwarding->val)++;    // 对栈上的 block_val 操作是通过 forwarding，此时其指向的是堆上的 block_val
blk.FuncPtr(blk);   // 堆 Block 内部操作的是自己的 block_val，其 forwarding 指向自己
```

![](https://user-gold-cdn.xitu.io/2017/1/19/037a2462d7a467a6ebab119237476a9e)

## 捕获对象
### `__strong` 类型的对象
```objc
int main()
{
    id array = [NSMutableArray new];
    void (^blk)(void) = ^{NSLog(@"array count = %ld",[array count]);};
    [array addObject:[NSObject new]];
    blk();    // 1
}
```

```objc
struct block_struct
{
    id __strong array;
    block_struct(void *fp, int _array):array(_array)
    {
        this.FuncPtr = fp;
    }
};

static void block_imp(block_struct *self)
{
    NSLog(@"array count = %ld",[self->array count]);
}

int main()
{
    id array = [NSMutableArray new];
    block_struct blk(block_imp, array);
    [array addObject:[NSObject new]];
    blk.FuncPtr(blk);   // 1
}
```

1. 截获对象时，Block 内部的成员变量是用 strong 修饰，会使变量的引用计数加 1
2. Block 释放时，自然也会对使该变量的引用计数 -1
3. 如果对这个 Block 进行 copy，那么栈 Block 会变成堆 Block，Block 本身的引用计数会 +1，不过这个例子没有体现

#### `__block` + `__strong` 类型的对象
```objc
int main()
{
    __block id array = [NSMutableArray new];
    void (^blk)(void) = ^{NSLog(@"array count = %ld",[array count]);};
    [array addObject:[NSObject new]];
    blk();    // 1
}
```

```c
struct block_val
{
    block_val *forwarding;
    id array;
};
```

和捕获 `__block` 一样，只是 `block_val` 内部维护的是一个对象



