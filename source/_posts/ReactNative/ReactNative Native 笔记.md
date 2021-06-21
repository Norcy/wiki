## Bridge 与线程
###  1. Bridge 中所有暴露的方法，分为同步方法和异步方法

```objc
// 这是同步方法的声明
RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(foo){}
// 这是异步方法的声明
RCT_EXPORT_METHOD(bar){}
```

### 2. Bridge 的方法调用，默认都在子线程

**默认情况下，Bridge 的方法都是从 JS 线程直接过来的，所以特别特别需要注意方法实现中不能去操作 UI**

如果需要操作 UI 有两种办法，第一种是直接异步到主线程，比如

```objc
RCT_EXPORT_METHOD(openView)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        // 这里是 UI 操作
    });
}
```

第二种方法如下，Bridge 中实现了该方法后，该 Bridge 的所有**异步方法**都默认在主线程调用

```objc
- (dispatch_queue_t)methodQueue
{
    return dispatch_get_main_queue();
}
```

**注意，这种情况下，如果是同步方法，依然会在 JS 线程中调用**
    
### 3. 导出常量

Native 暴露能力给 JS 调用，除了同步方法和异步方法，还有一种是导出常量

```objc
- (NSDictionary *)constantsToExport 
{
    return @{@"myKey": @"myValue"};
}
```

JS 中的调用方法如下

```js
// KRNBasic 是 Bridge 名称
console.log(NativeModules.KRNBasic.myKey);  // 输出 myValue
```


### 4. 指定线程去初始化函数和导出常量
如果你导出了常量，或者实现了 init 方法，RN 会提醒你实现 `+ (BOOL)requiresMainQueueSetup` 方法

该方法的作用是让 RN 引擎知道，你的 init 方法或 constantsToExport 方法在什么线程中执行。

如果你不实现这个方法，你的 init 或 constantsToExport 将会在子线程中执行


## Bridge 方法回调
```objc
RCT_EXPORT_METHOD(open:(id)obj success:(RCTResponseSenderBlock)success fail:(RCTResponseSenderBlock)fail)
```

每个方法最多支持两个 RCTResponseSenderBlock 参数，一个用于 success，一个用于 fail，且这两个方法只能回调一个

且无法将 RCTResponseSenderBlock 放到 Object 中

https://github.com/facebook/react-native/issues/9213