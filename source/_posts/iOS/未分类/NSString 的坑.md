## `__NSCFConstantString`

```objc
NSString *str1 = @"ab";
NSLog(@"%p isa: %@", str1, [str1 class]);
NSString *str2 = str1;
NSLog(@"%p isa: %@", str2, [str2 class]);
NSString *str3 = [[NSString alloc] initWithString:@"ab"];
NSLog(@"%p isa: %@", str3, [str3 class]);
NSString *str4 = [NSString stringWithFormat:@"ab"];
NSLog(@"%p isa: %@", str4, [str4 class]);
```

输出结果

```sh
0x1097013f8 isa: __NSCFConstantString
0x1097013f8 isa: __NSCFConstantString
0x1097013f8 isa: __NSCFConstantString
0xaeadd56e72cd6d4d isa: NSTaggedPointerString
```

其中 str1/str2/str3 的指向的内容地址相同，类都是 `__NSCFConstantString`

这是字符串常量，编译时分配内存，存储在常量区，引用计数是不会变的

而 str4 是一个 `__NSTaggedPointerString`

## `__NSTaggedPointerString`

这是苹果在 64 位环境下对 NSString,NSNumber 等对象做的一些优化。

64 位环境下指针变量长达 8 位，苹果把指针指向的内容直接放在了指针变量的内存地址中

这种指针不通过解引用 isa 来获取其所属类（通过其地址的部分保留字段），因此可以当作一种伪对象

这是一个单例常量，不会被释放。

对于 NSString 对象来讲，当非字面值常量的数字，英文字母字符串的长度小于等于 9 的时候会自动成为 NSTaggedPointerString 类型

如果有中文或其他特殊符号（可能是非 ASCII 字符）存在的话则会直接成为 `__NSCFString` 类型

```objc
NSString *str5 = [NSString stringWithFormat:@"abcdedfgh"];  // 长度<=9
NSLog(@"%p isa: %@", str5, [str5 class]);
NSString *str6 = [NSString stringWithFormat:@"abcdedfghi"]; // 长度>9
NSLog(@"%p isa: %@", str6, [str6 class]);
NSString *str7 = [NSString stringWithFormat:@"你"];          // 非 ASCII
NSLog(@"%p isa: %@", str7, [str7 class]);
```

输出结果

```sh
0x8166d7f2baa92699 isa: NSTaggedPointerString
0x600002be0240 isa: __NSCFString
0x600002e128c0 isa: __NSCFString
```


### 一道面试题

```objc
@property (nonatomic, copy) NSString *str;

int n = 100000;
while (n--)
{
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        self.str = [NSString stringWithFormat:@"abcdedfghi"];
    });
}
```

1. 会挂吗？会，因为是 nonatomic
2. 如果改为 atomic 会挂吗？不会
3. 如果只是赋值改为 @"a"，会挂吗？不会，因为变成了 `__NSTaggedPointerString`，不会调用其 setter

