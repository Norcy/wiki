## C++ 可以给参数设置默认值，当参数缺省的时候，使用默认值
如果没有设置默认值，则调用的时候不能省略参数

```cpp
void f(int i = 1, float f = 2.0f, double d = 3.0)
{
    cout << i << ", " << f << ", " << d << endl ;
}

int main(void)
{
    f() ; // 1, 2, 3
    f(10) ; // 10, 2, 3
    f(10, 20.0f) ; // 10, 20, 3
    f(10, 20.0f, 30.0) ; // 10, 20, 30
	return 0 ;
}
```

## 参数默认值可以是一个函数
```cpp
static int getValue()
{
    return 1;
}

int f(int a, int b = getValue())
{
    return b;
}
```


## 如果某个参数是默认参数，那么它后面的参数必须都是默认参数
所有有歧义的都是非法的

以下定义是非法的，因为有歧义。因为一旦只有两个参数，那么缺省的是第二个还是第三个呢？

```cpp
void f(int i, float f = 2.0f, double d);
```

## 函数重载时谨慎使用默认参数值
以下定义是非法的，因为有歧义。一旦只有一个参数，那么应该调用第一个函数还是第二个呢？

```cpp
int f(int a)
{
    return a;
}

int f(int a, int b = 1)
{
    return a + b;
}
```