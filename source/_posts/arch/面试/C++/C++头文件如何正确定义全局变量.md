## 背景
我们项目里看到一个头文件，是专门定义各种常量的，有一天发现这里的常量定义既使用了 `static`，又使用了 `const`，这个文件没有类，全部都是字符串的定义

```cpp
// QVNDefine.h
#ifndef QVNDefine_h
#define QVNDefine_h
const string VN_LIST = "list";
const string VN_CELL = "cell";
const string VN_VIEWPAPER = "view-pager";
static vector<string> VN_MULTICELLTYPE = {VN_LIST, VN_VIEWPAPER};
#endif /* QVNDefine_h */
```

此时我们有一番讨论，松哥觉得应该在头文件里声明，在实现文件里定义会更好，即类似

```cpp
// QVNDefine.h
#ifndef QVNDefine_h
#define QVNDefine_h
extern const string VN_LIST;
#endif /* QVNDefine_h */
```
```cpp
// QVNDefine.cpp
#include "QVNDefine.h"
extern const string VN_LIST = "list";
```

虽然是自己写的代码，但其实自己对这里的知识不太熟悉，习惯性就写了 `const`，又不知道为什么写了 `static`，所以找了一下资料系统学习一下

## 声明与定义
我们的问题是在 C++ 头文件中如何正确定义全局变量，首先要理解变量的声明和定义的区别是什么，翻了一下[之前的笔记](https://www.cnblogs.com/chenyg32/p/5080301.html)

```cpp
extern int a;       // 声明一个全局变量a
int a;              // 定义一个全局变量a
extern int a = 0;   // 定义一个全局变量a并给初值
int a = 0;          // 定义一个全局变量a并给初值
```

注意一下在 C++ 中

+ 定义只能有1处，但声明可以有多处，这就是 [ODR（定义与单一定义）规则](https://zh.cppreference.com/w/cpp/language/definition)
+ 定义引起内存分配，声明则不会
+ 注意变量的声明默认就是 extern

以上是针对变量的，如果是函数，还有点微妙的区别

函数的定义和声明一样是有区别的，定义函数要有函数体，声明函数没有函数体

所以与变量的区别就是没有函数体的函数是声明，而不是定义，如下例

```cpp
int a;              // 这是一个变量的定义
int fun(void);      // 这是一个函数的声明，省略了extern，完整些是extern int fun(void);
```

## 头文件定义全局变量几种方式的比较
根据以上信息，我们有以下几种定义全局变量的方式

### 第一种方法，直接定义
```cpp
// QVNDefine.h
#ifndef QVNDefine_h
#define QVNDefine_h
extern int a = 0;
#endif /* QVNDefine_h */
```
这是危险的，相当于 `int a = 0`，变量存放在同一个地址，是全局变量，多个实现文件包含该头文件是会发生重复定义问题，违背了 ODR 规则！

### 第二种方法，使用 static
```cpp
// QVNDefine.h
#ifndef QVNDefine_h
#define QVNDefine_h
static int a = 0;
#endif /* QVNDefine_h */
```
这是可行的，在编译阶段，每个包含该头文件的 .cpp 会生成一个 `static int a = 0`，变量存放在不同的地址，不是全局变量

### 第三种方法，使用 const
```cpp
// QVNDefine.h
#ifndef QVNDefine_h
#define QVNDefine_h
const int a = 0;
#endif /* QVNDefine_h */
```
这是可行的，在编译阶段，每个包含该头文件的 .cpp 会生成一个 `const int a = 0`，变量存放在不同的地址，不是全局变量，与 static 效果一样

### 第四种方法，使用 extern const 声明 + 实现文件定义
```cpp
// QVNDefine.h
#ifndef QVNDefine_h
#define QVNDefine_h
extern const int a;
#endif /* QVNDefine_h */
```
```cpp
// QVNDefine.cpp
#include "QVNDefine.h"
extern const int a = 1;
```
这是可行的，在编译阶段，其他包含该头文件的 .cpp 会生成一个 `extern const int a`，存放在同一个地址，是全局变量

除了方法一，其他方法都是可行的。那么它们有什么区别呢，谁才是最佳方式？

先说结论，大多数情况下，方法三最好

## 先聊聊 static
这里只阐述全局变量有无被 static 修饰的区别

一个全局变量（无论是定义在 .h 还是 .cpp），如果没有被 static 修饰，那么它是全局性的，假如该头文件被多次 include，在编译时就会产生重复链接的报错

而如果添加了 static，该全局变量就会变成静态全局变量，其作用域只在当前编译单元（比如 include 了该头文件的 .cpp）生效

所以实现文件的全局函数一般都要添加 static，这样不同的人编写不同的实现文件时，不用担心自己定义的函数，是否会与其它文件中的函数同名

**根据这个特点，假如有一百个实现文件包含了该头文件，那么这个全局变量就会被定义一百次，这个会造成内存空间的浪费，应该避免使用这种方式**

此时我们可以使用 extern 声明 + 实现文件定义的方法来解决多次定义的浪费问题

## 再聊聊 const
>const 的最初动机是取代预处理器 #define 来进行值替代，后来还被用于指针、函数变量、返回类型、类对象以及成员函数——《C++ 编程思想》 

这里我们只阐述头文件中的 const 有什么特点

const 在 C++ 中默认为内部链接（这一点与 C 不同，注意），即只对包含该定义的文件里是可见的，而不会被其他编译单元看到，故不是一个全局变量（与 static 类似），这个特点保证了不会有重复定义的错误

既然 const 与 static 类似，那么是否一样会有多次定义的浪费问题呢？以及为什么比方法四（extern 声明 + 实现文件定义）好？答案都在书里

>通常 C++ 编译器并不为 const 创建存储空间，相反它把这个定义保存在它的符号表里。大部分场合使用内部数据类型的情况，包括常量表达式，编译都能执行常量折叠——《C++ 编程思想》 

不过以下情况，编译器会进行存储空间的分配：

+ extern 成为 const 变量定义的一部分
+ 取一个 const 的地址
+ const 修饰的是一个复杂的对象

**如果 extern 成为 const 变量定义的一部分的时候，那么编译器会为强制进行了存储空间分配，extern 意味着使用外部连接，因此必须分配存储空间，这也就是说有几个不同的编译单元应当能够引用它，所以它必须存储空间**


### 常量折叠
何为常量折叠？[常量折叠](https://zh.wikipedia.org/wiki/%E5%B8%B8%E6%95%B8%E6%8A%98%E7%96%8A#%E5%B8%B8%E9%87%8F%E6%91%BA%E7%96%8A)（Constant folding）和常量传播（constant propagation）都是一种编译器最佳化技术

常量折叠表面上的效果和宏替换是一样的，只是 “效果上是一样的”，而两者真正的区别在于，宏是字符常量，在预编译完宏替换完成后，该宏名字会消失，所有对宏的引用已经全部被替换为它所对应的值，编译器当然没有必要再维护这个符号

而常量折叠发生的情况是，对常量的引用全部替换为该常量的值，但是，常量名并不会消失，编译器会把他放入到符号表中

```cpp
i = 320 * 200 * 32;
```
比如上面的代码中，编译器通常会在编译时直接计算出`320 * 200 * 32`的值，而不会在此生成2个乘法指令

## 结论
所以如果定义的都是内部数据类型，我们只要保证不对 const 变量进行取址操作（事实上也很少这样做），那么使用 const 的方式是最佳的，因为可以借助编译器的力量进行优化。最后回归背景问题，我们需要将 static 替换为 const 即可



## 参考文章
+ [Defining global constant in C++](https://stackoverflow.com/questions/2268749/defining-global-constant-in-c)
+ [const、static、extern 在头文件中似的情况小结](https://www.jianshu.com/p/b5ccc6036788)
+ 《C++ 编程思想》第二版 8.1.1 头文件里的 const
+ [C++常量折叠（这一篇关于分配空间的说法有失偏颇）](https://blog.csdn.net/yby4769250/article/details/7359278)
