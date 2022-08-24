## block 用什么修饰符修饰
对于这个问题，得区分 MRC 环境 和 ARC 环境；首先，通过上面小节可知，block 引用了普通外部变量，都是创建在栈区的；对于分配在栈区的对象，我们很容易会在释放之后继续调用，导致程序奔溃，所以我们使用的时候需要将栈区的对象移到堆区，来延长该对象的生命周期。

+ 对于 MRC 环境，使用 copy 修饰 block，会将栈区的 block 拷贝到堆区
+ 对于 ARC 环境，使用 strong、copy 修饰 block，都会将栈区的 block 拷贝到堆区

## 以下代码输出

```objc
- (void)blockDemo3
{
    NSMutableString *strM= [NSMutableString stringWithString:@"hello"];
	
    void (^block)() = ^ {
        [strM appendString:@"123"];
    };
	
    block();
}
```

输出 hello123，详细请看截获对象小节

> 随着 ARC 的完善，考察 Block 的类型判断跟考察 MRC 一样，越来越没有意义了，以下面试题可能过期

## 下面代码在 MRC 环境 和 ARC 环境运行的情况

```objc
void exampleA() {
	char a = 'A';
  	^{
   		printf("%cn", a);
  	}();
}
//调用：exampleA();
```
	
首先这个 Block 引用了普通外部变量，所以这个 Block 是在栈上面创建的；Block 是在 exampleA() 函数内创建的，然后创建完马上调用，这个时候 exampleA() 并没有执行完，所以这个栈 Block 是存在的，不会被 pop 出栈。故在 MRC 和 ARC 上面都可以正确执行
	
## 下面代码在 MRC 环境 和 ARC 环境运行的情况

```objc
void exampleB_addBlockToArray(NSMutableArray *array) {
  char b = 'B';
  [array addObject:^{
    printf("%cn", b);
  }];
}
	
void exampleB() {
  NSMutableArray *array = [NSMutableArray array];
  exampleB_addBlockToArray(array);
  void (^block)() = [array objectAtIndex:0];
  block();
}
	
//调用：exampleB()
```
	
ARC 正常，MRC 崩溃。修复方法就是加 copy，如果不懂点[这里](https://ioscaff.com/articles/221)

## 下面代码在 MRC 环境 和 ARC 环境运行的情况

```objc
void exampleC_addBlockToArray(NSMutableArray *array) {
    array addObject:^{
        printf("Cn");
    }];
}
	
void exampleC() {
    NSMutableArray *array = [NSMutableArray array];
    exampleC_addBlockToArray(array);
    void (^block)() = [array objectAtIndex:0];
    block();
}
	
//调用：exampleC();
```
	
全局 Block，没有任何问题

## 下面代码在 MRC 环境 和 ARC 环境运行的情况

```objc
typedef void (^dBlock)();
dBlock exampleD_getBlock() {
  char d = 'D';
  return ^{
    printf("%cn", d);
  };
}
void exampleD() {
  exampleD_getBlock()();
}
//调用：exampleD();
```
	
MRC 编译器可检查出来，会编译失败；ARC 没问题

## 下面代码在 MRC 环境 和 ARC 环境运行的情况

```objc
typedef void (^eBlock)();
eBlock exampleE_getBlock() {
  char e = 'E';
  void (^block)() = ^{
    printf("%cn", e);
  };
  return block;
}
void exampleE() {
  eBlock block = exampleE_getBlock();
  block()
}
//调用：exampleE();
```
	
MRC 编译通过，调用异常；ARC 没问题

## ARC 环境下输入结果

```objc
 __block NSString *key = @"AAA";

objc_setAssociatedObject(self, &key, @1, OBJC_ASSOCIATION_ASSIGN);
id a = objc_getAssociatedObject(self, &key);

void (^block)(void) = ^ {
    objc_setAssociatedObject(self, &key, @2, OBJC_ASSOCIATION_ASSIGN);
};

id m = objc_getAssociatedObject(self, &key);
block();
id n = objc_getAssociatedObject(self, &key);
objc_setAssociatedObject(self, &key, @3, OBJC_ASSOCIATION_ASSIGN);
id p = objc_getAssociatedObject(self, &key);
NSLog(@"%@ --- %@ --- %@ --- %@",a,m,n,p);
```
    
答：输入结果：1 --- (null) --- 2 --- 3，代码执行过程如下：

+ __block 修饰的 key，创建在栈区，访问变量 key 为：&(结构体->forwarding->key) ，key 在栈区，此时利用栈区地址作为 Key 来存值
+ 变量 a 使用栈区地址取值，故 a 的值为 1
+ 声明一个 block，引用到了外部变量 key，此时将 block 从栈拷贝堆，访问变量 key 为：&(结构体->forwarding->key) ，key 在堆区
+ 变量 m 用堆区地址来取值，故为 null
+ 执行 block，用堆区地址将 2 存进去
+ 变量 n 用堆区地址来取值，故为 2
+ 再用堆区地址将 3 存进去
+ 变量 p 用堆区地址来取值，故为 3

## 使用block和使用delegate完成委托模式有什么优点
委托模式在设计模式中是适配器模式中的对象适配器，Objective-C 中使用 id 类型指向一切对象，使委托模式在 iOS 中的实现更为方便

使用block实现委托模式，其优点是回调的 block 代码块定义在委托对象函数内部，使代码更为紧凑

适配对象不再需要实现具体某个 protocol，代码更为简洁

## 以下代码有问题吗

```objc
int a = 7;
void (^myBlock)(void) = ^() {
    a = 8;
};
printf("%d", myBlock());
```

有问题，会出现 2 处编译错误，第一处是 a 不能被赋值，除非添加 `__block`；第二处是 myBlock 没有返回值，无法被打印

## 以下代码有问题吗
```objc
id ret;
dispatch_sync(self.concurrentQueue, ^{
    ret = @"1";
});
NSLog(@"%@", ret)
```

会有编译错误，必须添加 `__block`，这样对象才可以在 block 中被重新赋值