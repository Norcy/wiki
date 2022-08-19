## Weak-Strong Dance
```objc
__typeof(&*self) __weak weakSelf = self;
self.completionHandler = ^(NSInteger result) {
    [weakSelf.property removeObserver:weakSelf forKeyPath:@"pathName"];
};
```

假设 block 被放在子线程中执行，而且执行过程中 self 在主线程被释放了。由于 weakSelf 是一个弱引用，因此会自动变为 nil

而这个例子的 KVO 中，这会导致崩溃

解决以上问题的方法很简单，新增一行代码即可

```objc
__typeof(&*self) __weak weakSelf = self;
self.completionHandler = ^(NSInteger result) {
    __typeof(&*self) __strong strongSelf = weakSelf;
    [strongSelf.property removeObserver:strongSelf forKeyPath:@"pathName"];
};
```

self 所指向对象的引用计数变成 2，即使主线程中的 self 因为超出作用于而释放，对象的引用计数依然为 1，避免了对象的销毁

1. Q：下面这行代码，将一个弱引用的指针赋值给强引用的指针，可以起到强引用效果么？

    `__typeof(&*self) __strong strongSelf = weakSelf;`

    A：会的。引用计数描述的是对象而不是指针。strongSelf 会强引用 weakSelf 指向的那个对象。因此对象的引用计数会加一

2. Q：block 内部定义了 strongSelf，会不会因此强引用了 strongSelf？

    A：不会。block 只有截获外部变量时，才会引用它。如果是内部新建一个，则没有任何问题

3. Q：如果在 block 内部没有强引用，而是通过 if 判断，是不是也可以，比如这样写：

    ```objc
    __typeof(&*self) __weak weakSelf = self;
    wself.completionHandler = ^(NSInteger result) {
        if (weakSelf)
        {
            [weakSelf.property removeObserver:weakSelf forKeyPath:@"pathName"];
        }
    };
    ```

    A：不可以！考虑到多线程执行，也许在判断的时候，self 还没释放，但是执行 self 里面的代码时，就刚好释放了

4. Q：那按照这个说法，block 内部强引用也没用啊。也许 block 执行以前，self 就释放了

    A：有用！如果在 block 执行以前，self 就释放了，那么 block 的引用计数降为 0，所以自己就会被释放。这样它根本就不会被执行

5. Q：如果在执行 block 的过程中，block 被释放了怎么办？

    A：简单来说，block 还会继续执行，但是它捕获的指针会变成 nil

    ```objc
    __weak ObjectA * weakSelf = self;
    self.completion = ^{
        weakSelf.completion = nil;
        [weakSelf doSomethingElse];
    };
    ```

    当我们这样调用时，执行到 `weakSelf.completion = nil` 时，block 会被释放但会继续执行，weakSelf 会变为 nil，因此 doSomethingElse 不会被执行

    ``` objc
    _completion(); // 直接使用成员变量
    ```

    如果这样调用就可以避免 block 的释放，这个会在栈上创建一个 block 的 copy，原始的 block 对象会被释放，但是新的对象会继续正常执行，weakSelf 也不会变 nil

    ```objc
    self.completion();
    ```

## 使用系统 API 需不需要使用 weak
正常情况下是不需要的，比如

```objc
[UIView animateWithDuration:duration
                     animations:^{
                     [self.superview layoutIfNeeded];
                 }];

[[NSOperationQueue mainQueue] addOperationWithBlock:^{
    self.someProperty = xyz;
}];

[[NSNotificationCenter defaultCenter] addObserverForName:@"someNotification"
                                                  object:nil
                                                   queue:[NSOperationQueue mainQueue]
                                              usingBlock:^(NSNotification *notification) {
                                              self.someProperty = xyz;
                                          }];
```

但是如果系统的 API 对 self 有引用的时候就要考虑，比如

```objc
// self->operationsGroup/operationsQueue->block->self
__weak __typeof__(self) weakSelf = self;
dispatch_group_async(_operationsGroup, _operationsQueue, ^{
    __typeof__(self) strongSelf = weakSelf;
    [strongSelf doSomething];
    [strongSelf doSomethingElse];
});

// self->observer->block->self
__weak __typeof__(self) weakSelf = self;
_observer = [[NSNotificationCenter defaultCenter] addObserverForName:@"testKey"
            object:nil
             queue:nil
        usingBlock:^(NSNotification *note) {
      __typeof__(self) strongSelf = weakSelf;
      [strongSelf dismissModalViewControllerAnimated:YES];
}];
```

## 封装常用的宏
```objc
#define WS __typeof(&*self) __weak weakSelf = self
#define SS __typeof(&*self) __strong strongSelf = weakSelf
```