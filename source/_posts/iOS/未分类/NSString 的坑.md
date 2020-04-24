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

## NSString 与引用计数
+ `__NSCFConstantString` 的引用计数无限大
+ `__NSTaggedPointerString` 的引用计数无限大
+ `__NSCFString` 的引用计数正常，对一个 `__NSCFString` 进行 copy 操作会使得该对象的引用计数 +1

> 可以通过 `po @(CFGetRetainCount((__bridge CFTypeRef)(s)))` 查看其引用计数

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

可能挂在这里

```sh
#0  0x00007fff51a9033a in __pthread_kill ()
#1  0x00007fff51b3ae60 in pthread_kill ()
#2  0x00007fff51a1fb7c in abort ()
#3  0x00007fff51b1ba63 in malloc_vreport ()
#4  0x00007fff51b1bde6 in malloc_zone_error ()
#5  0x00007fff23d9f945 in _CFRelease ()
#6  0x000000010dcd4e0f in -[ViewController setStr:]
```

原因是多个线程调用 string 的 setter 时，当 `_str` 引用计数为 1 时，release 被调用了，过度释放造成 crash

```objc
- (void)setStr:(NSString *)str
{
    if (_str != str)
    {
        [_str release]; // arc 自动加上
        _str = [str copy];
    }
}
```

+ 改为 `self.str = [NSString stringWithFormat:@"abcdedfgh"];` 就不会挂了，因为 `__NSTaggedPointerString` 的引用计数无限大，多次 release 也没事
+ 同理，改为 `self.str = @"abcdedfgh"` 也不会挂，因为 `__NSCFConstantString` 的引用计数无限大
+ 改为 atomic 也可以防止 crash

另外在 autoreleasepool pop 的时候也会调用 release，也可能会挂，堆栈如下

```sh
#0  0x00007fff50aed94b in objc_release ()
#1  0x00007fff50aef077 in AutoreleasePoolPage::releaseUntil(objc_object**) ()
#2  0x00007fff50aeef96 in objc_autoreleasePoolPop ()
#3  0x0000000101ed1e77 in _dispatch_last_resort_autorelease_pool_pop ()
#4  0x0000000101ee3825 in _dispatch_root_queue_drain ()
#5  0x0000000101ee3ca6 in _dispatch_worker_thread2 ()
#6  0x00007fff51b379f7 in _pthread_wqthread ()
#7  0x00007fff51b36b77 in start_wqthread ()
```



