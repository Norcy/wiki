---
title: dynamic_cast
---

C++ 中

+ 如果是子类指针强制转换父类指针，则称为上行转换（upcasting）
+ 如果是父类指针强制转换子类指针，则称为下行转换（downcasting）

# 例子

```c++
classB
{
public:
    int m_iNum;
    virtual void foo();
};

classD : public B
{
public:
    char *m_szName[100];
};

void func(B *pb)
{
    D *pd1 = static_cast<D *>(pb);
    D *pd2 = dynamic_cast<D *>(pb);
}
```

在上面的代码段中，如果 pb 指向一个 D 类型的对象，pd1 和 pd2 是一样的，并且对这两个指针执行 D 类型的任何操作都是安全的；

但是，如果 pb 指向的是一个 B 类型的对象，那么 pd1 将是一个指向该对象的指针，对它进行 D 类型的操作将是不安全的（如访问 m_szName），而 pd2 将是一个空指针，因而 `dynamic_cast` 更安全

## 结论

如果进行上行转换，则使用 `dynamic_cast` 和 `static_cast` 的效果一样，都能得到正确的转换结果

如果进行下行转换，`dynamic_cast` 具有类型检查的功能，比 `static_cast` 更安全；如果类的关系是对的，则转换成功，返回一个指向子类的指针，否则返回 NULL 指针

# 注意
`dynamic_cast` 转化时，如果父类不是一个多态类，也就是没有虚函数，那么编译失败，因此如果要实现 `dynamic_cast` 的转换，至少把父类的析构函数声明为虚函数（不必须是纯虚函数）；而 `static_cast` 会编译成功，不安全

为什么需要虚函数呢：从意义上说，类中存在虚函数，就说明它有想要让基类指针或引用指向派生类对象的情况，此时转换才有意义。从技术上说，这是由于运行时类型检查需要运行时类型信息，而这个信息存储在类的虚函数表中，只有定义了虚函数的类才有虚函数表
