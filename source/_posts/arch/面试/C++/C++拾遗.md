## 虚函数相关
+ 定义一个函数为虚函数，不代表函数为不被实现的函数。
+ 定义基类的函数为虚函数是为了允许用基类的指针来调用子类的这个函数（多态）
+ 定义一个函数为纯虚函数，才代表函数没有被实现，规范继承这个类的子类必须实现这个函数

+ 抽象类的定义：称带有纯虚函数的类为抽象类 
+ 抽象类无法实例化
+ 继承抽象类的子类如果没有实现纯虚函数，则该子类也是抽象类

## 虚析构函数 v.s. 纯虚析构函数
1. 实现多态时，父类的析构函数必须为虚析构函数，否则 delete 的时候父类的析构函数不会被调用
2. 如果想实现一个抽象类，如果其中没有其他合适的函数，可以把析构函数定义为纯虚的
3. 纯虚函数通常都不需要函数体，因为我们一般不会调用抽象类的这个函数，但是不代表不能有函数体
4. 无论是虚析构函数还是纯虚析构函数，都必须有函数体。因为即使是抽象类，其析构函数也应该需要被调用到（会被子类的析构函数隐式调用到）

```cpp
class Base
{
public:
   Base(){}
   virtual ~Base()= 0;
};
```

这段代码会编译错误，因此析构函数一定需要函数体，但是对于纯虚函数不能把实现和 `=0` 写在一起，因此需要改为这样

```cpp
class Base
{
public:
   Base()
   {
   }
   virtual ~Base() = 0;
};

Base::~Base()
{
} 
```


## virtual 的隐式继承
virtual 修饰符是会隐式继承的：基类是 virtual 函数，子类不写 virtual 关键字，其函数依然是 virtual

## `delete []` 和 delete
+ 数组对象使用 delete 的话，虽然这块内存会正确地归还，但只会针对数组内的第一个对象执行析构函数，后续的对象的析构函数都无法被执行
+ 基本类型的对象没有析构函数，所以回收基本类型组成的数组空间用 delete 和 `delete[]` 都是应该可以的
+ 但是对于类对象数组，只能用 delete[]。对于 new 的单个对象，只能用 delete 不能用 delete[] 回收空间
+ 所以一个简单的使用原则就是：new 和 delete、new[] 和 delete[] 对应使用


## 混编
1. OC 的 .h 和 .m 包含 C++ 类，它作为属性
2. C++ 的 .h 和 .cpp 包含 OC 类，它作为变量
3. C++类与 void*的转换，OC 类与 void*的转换
4. OC 类在 C++ 之间传递，用 void *、bridge【QVNImplementationUtils】
5. C++ 类在 OC 之间传递，用 void *
6. C++ 类，有一个 OC 变量，OC 的生成和释放都是 ARC 管理的

## 写了 inline 就一定会内联吗？不写 inline 一定不会内联吗
inline 仅仅是一个对编译器的建议，最后能否真正内联取决于编译器，它如果认为你的函数不复杂，能在调用点展开，并不是说声明了内联就会内联；同理，没有声明 inline 的也不一定不会内联，如下代码

比如以下代码极有可能会被内联

```cpp
static int Inc(int i) {return i+1};
int i;
for (i=0; i<999999; i = Inc(i)) {};
```

变为

```cpp
static int Inc(int i) {return i+1};
int i;
for (i=0; i<999999; i = i+1) {};
```

## inline 写在函数声明的地方是无效的
如下风格的函数 Foo 不能成为内联函数：

```cpp
inline void Foo(int x, int y); // inline 仅与函数声明放在一起
void Foo(int x, int y){}
```

而如下风格的函数Foo 则成为内联函数：

```cpp
void Foo(int x, int y);
inline void Foo(int x, int y) {}// inline 与函数定义体放在一起
```

所以说，inline 是一种 “用于实现的关键字”，而不是一种 “用于声明的关键字”

## 全局的 inline 函数要把实现写在头文件
首先 inline 写在函数声明处是无效的；其次如果只在 .c 中声明 inline，.h 不声明，那么其他文件引用该 .h 的时候，根本不知道该函数是一个内联函数，所以只能把实现和 inline 关键字写在头文件

其次，定义在头文件中的 inline 需要配合 static

如果没有 static，那么编译器会认为该内联函数是全局的，会把该函数进行编译（即当成普通的全局函数而不是在调用处展开），那么就失去了内联的效果

https://medium.com/@hauyang/%E6%88%91%E6%9C%89%E6%89%80%E4%B8%8D%E7%9F%A5%E7%9A%84-static-inline-b363892b7450

## C++ 的枚举写法有坑
```cpp
vector<vector<int>> ret;

// 这种写法，修改 v 不会影响 ret，因为 v 是拷贝构造了
for (auto v : ret)
    v.pop_back();

// 这种写法，修改 v 会影响 ret，因为 v 是引用
for (auto &v : ret)
    v.pop_back();

// 加引用的写法等同于这个写法
for (int i = 0; i < ret.size(); ++i)
    ret[i].pop_back();
```

## [C++变量的初始化](https://harttle.land/2015/10/05/cpp-variable-init.html)

+ 全局变量、静态变量：0
+ 局部变量：随机
+ 类的成员变量
    + 全局：0
    + 静态：0
    + 局部：随机

```cpp
int g_var;
int *g_pointer;
static int g_static;

int main(){
    int l_var;
    int *l_pointer;
    static int l_static;

    cout<<g_var<<endl<<g_pointer<<endl<<g_static<<endl; // 0 0 0 
    cout<<l_var<<endl<<l_pointer<<endl<<l_static<<endl; // 非0 非0 0
};
```

```cpp
class A{
public:
    int v;
};
A g_var;

int main(){
    A l_var;
    static A l_static;
    cout<<g_var.v<<' '<<l_var.v<<' '<<l_static.v<<endl; // 0 2407223 0
    return 0;
}
```

## 一个类型转换的问题（待解决）
```cpp
class A
{
public:
    virtual void doA() = 0;
    virtual ~A(){printf("~A\n");};
};

class B
{
public:
    virtual void doB() = 0;
    virtual void doC() = 0;
    virtual ~B(){printf("~B\n");};
};

class SubClass: public A, public B
{
public:
    void doA() override;
    void doB() override;
    void doC() override;
    ~SubClass(){printf("~SubClass\n");};
};

void SubClass::doA(){printf("doA\n");};
void SubClass::doB(){printf("doB\n");};
void SubClass::doC(){printf("doC\n");};
```

测试代码如下

```cpp
A *a = new SubClass();
a->doA();           // doA
B *b = (B *)a;
b->doB();           // doA
b->doC();           // ~SubClass ~B ~A

A *a = new SubClass();
a->doA();           // doA
B *b = (B *)a;
b->doC();           // ~SubClass ~B ~A
b->doB();           // Crash:libc++abi.dylib: Pure virtual function called!

A *a = new SubClass();
a->doA();           // doA
SubClass *sub = dynamic_cast<SubClass *>(a);
sub->doB();         // doB
sub->doC();         // doC

A *a = dynamic_cast<A*>(new SubClass());
a->doA();           // doA
B *b = dynamic_cast<B*>(a);
b->doB();           // doB
b->doC();           // doC
```

`A *a = new SubClass();` 这种写法和 `A *a = dynamic_cast<A*>(new SubClass());` 本质上没有区别

但是 `B *b = (B *)a;` 和 `SubClass *sub = dynamic_cast<SubClass *>(a);` 是有区别的

注意以上代码都不应该打印析构函数的内容，因为没有调用 delete