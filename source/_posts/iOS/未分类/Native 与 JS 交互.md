## UIWebView/WKWebView 拦截请求

1. Native 调用 JS

```objc
[webView stringByEvaluatingJavaScriptFromString:@"Math.random();"];
```

2. JS 调用 Native
Native 拦截 UIWebView 的所有请求，判断 Scheme，如果是约定好的 Schema 就拦截请求、解析参数并调用 Native 相应的逻辑

JS 发起请求有两种方式：1. 通过 localtion.href；2. 通过 iframe 方式

前者如果短时间内连续多次修改 localtion.href 的值，Native 只会收到最后一次请求，因此 JS 侧采用 iframe 的方式发起请求

```js
var url = 'jsbridge://doAction?title=标题';
var iframe = document.createElement('iframe');
iframe.style.width = '1px';
iframe.style.height = '1px';
iframe.style.display = 'none';
iframe.src = url;
document.body.appendChild(iframe);
setTimeout(function() {
    iframe.remove();
}, 100);
```

```objc
- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
    if (request.URL 是自定义的 Schema)
    {
        解析 URL 的参数
        处理函数
        return NO;  // 不发起请求
    }

    return YES; // 正常发起请求
}
```


+ 优点：不需要等到整个 HTML 页面加载完成就能调用 Native？
+ 缺点：需要繁琐地解释字符串得到相应的方法名和传值，且调用的方法也不能传递返回值；

## UIWebView 获取 JSContext
在 webViewDidFinishLoad 通过 KVC 的方法获取 JSContext

```objc
- (void)webViewDidFinishLoad:(UIWebView *)webView
{
    self.jsContext = [webView valueForKeyPath:@"documentView.webView.mainFrame.javaScriptContext"];
    self.jsContext[@"NativeBridge"] = self;
    self.jsContext.exceptionHandler = ^(JSContext *context, JSValue *exceptionValue) {
        context.exception = exceptionValue;
        NSLog(@"异常信息：%@", exceptionValue);
    };
}
```

JSContext 属于 JSCore，见下节

+ 优点：需要等到整个 HTML 页面加载完成才能调用 JS？
+ 缺点：WKWebView 无法使用 JSCore

## WKWebView 使用 userContentController
WKWebView 无法使用 JSCore

1. Native 调用 JS

```objc
@property (nonatomic, strong) WKUserContentController *userContentController;
```

Native 和 H5 交互基本全靠这个对象， 在 WKWebVeiw 中，我们使用我们有两种方式来调用 JS，

+ 使用 WKUserScript
+ 直接调用 JS 字符串

1.1 使用 WKUserScript

```objc
// source 就是我们要调用的 JS 函数或者我们要执行的 JS 代码
// injectionTime 这个参数我们需要指定一个时间，在什么时候把我们在这段 JS 注入到 WebVeiw 中，它是一个枚举值，WKUserScriptInjectionTimeAtDocumentStart 或者 WKUserScriptInjectionTimeAtDocumentEnd
// MainFrameOnly 因为在 JS 中，一个页面可能有多个 frame，这个参数指定我们的 JS 代码是否只在 mainFrame 中生效
- initWithSource:injectionTime:forMainFrameOnly:
```

至此，我们已经构建了一个 WKUserScript，然后呢，我们要做的就是要把它添加进来

```objc
- addUserScript:
```

至此使用 WKUserScript 调用 JS 完成

1.2 直接调用 JS 字符串

在 WKWebView 中，我们也可以直接执行 JS 字符串

```objc
- (void)evaluateJavaScript:completionHandler:
```

我们通过调用这个方法来执行 JS 字符串，然后在 completionHandler 中拿到执行这段 JS 代码后的返回值。

至此，Native 调用 JS 完成


2. JS 调用 Native

2.1 向 JS 注入一个字符串

```objc
[_webView.configuration.userContentController addScriptMessageHandler:self name:@"nativeMethod"];
```

我们向 JS 注入了一个方法，叫做 nativeMethod

2.2 JS 调用 Native

```js
window.webkit.messageHandlers.nativeMethod.postMessage(value);
```

一句话调用，我们就可以在 Native 中接收到 value

2.3 接收 JS 调用

上边我们调用 addScriptMessageHandler:name 的时候，我们要遵守 WKScriptMessageHandler 协议，然后实现这个协议。

```objc
- (void)userContentController:(WKUserContentController *)userContentController didReceiveScriptMessage:(WKScriptMessage *)message 
{
    NSString * name = message.name  // 就是上边注入到 JS 的哪个名字，在这里是 nativeMethod
    id param = message.body         // 就是 JS 调用 Native 时，传过来的 value
    // TODO: do your stuff
}
```

完了，Native 调用 JS 就这么简单

优点：简单易用
缺点：JS 调用 Native 后回调较难，见 https://juejin.im/entry/59f6e836f265da431a427a57

## JSCore

1. Native 调用 JS

```objc
JSContext *context = [[JSContext alloc] init];
NSString *js = @"function add(a,b) {return a+b}";
[context evaluateScript:js];    // 注入 JS 函数
[context[@"add"] callWithArguments:@[@"2", @"3"]];
//[mJsValue invokeMethod:@"printHello" withArguments:@[@"1"]];
```

2. JS 调用 Native

借助 JSCore，我们并不一定要写 JS，可以直接使用 JSCore 模拟 JS 调用

```objc
JSContext *context = [[JSContext alloc] init];
[context evaluateScript:@"var a = 1;var b = 2;"];
JSValue *ret = [context evaluateScript:@"a + b"];
NSInteger sum = [ret toInt32]; // sum=3
```

Native 注入函数到 JS

```objc
JSContext *context = [[JSContext alloc] init];
context[@"NativeBridge"] = [QLJSInterface new];
```

Native 实现 JSExport

```objc
@protocol QLJSProtocol <JSExport>
JSExportAs(open, - (void)open:(NSString *)url);
@end

@interface QLJSInterface : NSObject<QLJSProtocol>
@end
@implementation QLKTH5Interface
- (void)open:(NSString *)url
{
    NSLog(@"URL is %@": url);
}
@end
```

JS 调用 Native，可以使用 JSCore 模拟，也可以在 JS 侧调用（需要把 JS 文件注入到 JSContext）

```objc
JSContext *context = [[JSContext alloc] init];
context[@"NativeBridge"] = [QLJSInterface new];
[context evaluateScript:@"NativeBridge.open('HomePage')"];
```

or

```js
function f()
{
    NativeBridge.open('HomePage');
}
```

+ 优点：不依赖 WebView