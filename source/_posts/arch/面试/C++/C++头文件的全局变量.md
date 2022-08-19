## 变量的定义与声明
```cpp
extern int a;       // 声明一个全局变量a
int a;              // 定义一个全局变量a
extern int a = 0;   // 定义一个全局变量a并给初值
int a = 0;          // 定义一个全局变量a并给初值
```

>
+ 定义只能有1处，但声明可以有多处
+ 定义引起内存分配，声明则不会
+ 注意变量和函数的声明默认就是 extern

## 函数的定义与声明
函数的定义和声明是有区别的，定义函数要有函数体，声明函数没有函数体，所以函数定义和声明时都可以将 extern 省略掉

```cpp
int a;              // 这是一个变量的定义
int fun(void);      // 这是一个函数的声明，省略了extern，完整些是extern int fun(void);
```

## 头文件中的全局变量

```cpp
// test.h
#ifndef _TEST_H_  
#define _TEST_H_  
// 这是可行的，在编译阶段，每个包含 test.h 的文件会生成一个 static int a = 0; 
// 存放在不同的地址，不是全局变量
static int a = 0;

// 这是可行的，在编译阶段，每个包含 test.h 的文件会生成一个 const int b = 0; 
// 存放在不同的地址，不是全局变量，与 static 一样
const int b = 1;

// 这是可行的，在编译阶段，每个包含 test.h 的文件会生成一个 extern int c; 
// 存放在同一个地址，是全局变量  
// 使用的时候必须在其中一个".cpp"文件定义 int c; 之后才可以正确运行
// 因为 extern 的含义是变量定义在别的位置
extern int c;

// 这是危险的，相当于int d = 0
// 存放在同一个地址，是全局变量  
// 多个头文件包含是会发生重定义问题！！！
extern int d = 0;

// vector 同理
// p.s. 对于 const 的迭代器访问，需要使用 const_iterator 而不是 iterator
const std::vector<std::string> VN_MULTICELLTYPE = {"1", "2"};
#endif
```

1. 注意全局 const 默认为该编译单元的局部变量（内部链接性），即类似 static 修饰（C 和 C++ 不同，const 常量在 C 中依旧是外部链接性的 ）
2. 在头文件中定义 static/const 变量会造成变量多次定义，造成内存空间的浪费，而且也不是真正的全局变量。应该避免使用这种定义方式
3. 正确的方式是在头文件中使用声明，在某一个实现文件中定义，其他实现文件引用即可

```cpp
//tesh.h
#ifndef _TEST_H_
#define _TEST_H_
extern int a;   // 此处声明
#endif

// a.cpp
#include "test.h"
a = 2;      // 此处定义
cout << a << endl;

// b.cpp
#include "test.h"
cout << a << endl;  // 输出2
```

## 参考文章
+ [NSNotification Name 最佳写法](https://www.cnblogs.com/chenyg32/p/5080301.html)
+ [const、static、extern 在头文件中似的情况小结](https://www.jianshu.com/p/b5ccc6036788)
+ [勿在头文件中定义static变量](https://blog.csdn.net/yockie/article/details/50768753)
