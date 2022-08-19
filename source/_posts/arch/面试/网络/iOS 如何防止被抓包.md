## 背景
想抓`订阅号`助手的请求，看能否实现公众号推文自动发布，结果用 Stream 抓包软件抓不到任何的请求，但是请求却正常发送了

## 疑问
如何既能正常发出请求，又能不被抓包软件（如 Stream）抓到任何请求

## 方法
只要 Hook NSURLSessction 的方法，将其中 configuration 的 connectionProxyDictionary 清空即可

其中部分关键代码如下


```objc
+ (NSURLSession *)dp_sessionWithConfiguration:(NSURLSessionConfiguration *)configuration
                                     delegate:(nullable id<NSURLSessionDelegate>)delegate
                                delegateQueue:(nullable NSOperationQueue *)queue
{
    if (!configuration)
    {
        configuration = [[NSURLSessionConfiguration alloc] init];
    }
    if (isDisableHttpProxy)
    {
        configuration.connectionProxyDictionary = @{};
    }
    return [self dp_sessionWithConfiguration:configuration delegate:delegate delegateQueue:queue];
}

+ (NSURLSession *)dp_sessionWithConfiguration:(NSURLSessionConfiguration *)configuration
{
    if (configuration && isDisableHttpProxy)
    {
        configuration.connectionProxyDictionary = @{};
    }
    return [self dp_sessionWithConfiguration:configuration];
}
```

详细代码请见 [https://github.com/Norcy/DisableHttpProxy](https://github.com/Norcy/DisableHttpProxy)