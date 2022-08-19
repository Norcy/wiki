为了适应多种多样的业务需求，VideoNative 提供了不同维度的 VN 页面加载方式，主要包括三大类：单纯的单页面模式、App 模式和 App 的单页面模式

## 单纯的单页面模式
### 1. 简介
这是单纯的单页面模式，是最灵活的 VN 页面加载方式，业务从 VN 框架获取到 QVNPage 对象之后，调用 getView 方法可以获取到一个 UIView。这个模式下，VN 框架扮演的是一个 View 的提供者，至于 View 的大小、添加、删除等都交给业务方灵活处理

该模式优点是非常灵活，对工程代码的侵入性小，但是无法使用以下接口

+ vn.navigate
+ vn.app
+ vn.window

涉及的关键类： QVNPage

### 2. 接口
```objc
// QVNVideoNative.h
- (void)loadSinglePage:(NSString *)rootUrl
               pageUrl:(NSString *)pageUrl
              callback:(IVNLoadPageCallback)callback;
```

我们可以通过 QVNVideoNative 单例加载单页面。不同于通过 VNApp 加载页面，本方法不需要 AppId，而是直接通过路径来加载页面

+ rootUrl 是存放该页面源码的根目录，图片资源、跳转链接等路径会以此路径做转换
+ pageUrl: 页面相对于 RootUrl 的路径，注意不是以 vn:// 开头
+ callback: 加载单页面成功的回调，其定义如下

```objc
typedef void (^IVNLoadPageCallback)(QVNPage *page,
                                    NSString *appId,
                                    NSString *rootUrl,
                                    NSString *pageUrl,
                                    QVNErrorCode errorCode);
```

其中回调参数最重要的是 QVNPage，调用者需要强持有 page 对象，否则会被释放

> 对于单页面，似乎提供一个绝对路径能够读取到对应的 .page 即可，其实不然，rootUrl 是必须的，因为 VN 源码中的图片路径和跳转链接等等都是相对路径，外部需要提供一个绝对的根目录，以计算出图片和跳转等的绝对路径；而 pageUrl 是相对 rootUrl 的路径，两者结合可以计算出 .page 的绝对路径

### 3. 使用范例

MyApp 的 VN 代码存放在 Bundle 中，以下图为例，vnapp 是存放着 VN 代码的目录

```
MyApp.app
└── vnapp
    ├── image
    │   └── image1.png
    └── index
        ├── index.page
        └── test.page
```

下面展示如何加载 index.page 这个单页面，并添加到视图中

```objc
NSString *rootUrl = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"vnapp"];
[[QVNVideoNative sharedInstance] loadSinglePage:rootUrl pageUrl:@"index/index" callback:^(QVNPage *page, NSString *appId, 
NSString *rootUrl, NSString *pageUrl, QVNErrorCode errorCode) {
    self.page = page;
    [self.containerView addSubview:[self.page getView]];
}];
```

如果要获取 test.page，则 pageUrl 改为 @"index/test"

> 注意，在单纯的单页面模式中，没有 App/AppId 的概念

> 注意，这里的 pageUrl 是一个相对路径


## App 模式
### 1. 简介
单纯的单页面模式只提供了一个 View，而 QVNApp 模式就相对更重些，它提供了 Controller 和其跳转的功能。QVNApp 模式下，VN 框架接管了整个 Controller，顶部的状态栏，以及 Controller 的跳转。

如果采用 vn.navigateTo 等框架提供的跳转方法，那么跳转到的将会是 QVNApp 下的另一个 Controller，当然开发者也可以采用 jsapi 的方式，从 VN 页面跳转到其他原生 Controller

该模式适用于有多个 VN 页面且它们比较集中的业务场景，可以类比微信的小程序，也适用于使用 VN 技术构建整个 App；但是对工程的侵入性较大，比如接管了跳转和顶部状态栏、使用的 Controller 是 VN 框架内部的，外部无法干预等

涉及的关键类： QVNApp

### 2. 接口

1. 获取 QVNApp 的接口

	```objc
	// QVNVideoNative.h
	- (QVNApp *)getVNApp:(NSString *)appId rootUrl:(NSString *)rootUrl;
	```
		
	我们可以通过 QVNVideoNative 单例获取到 QVNApp
	
	+ appId: 每个 QVNApp 都有 AppId 作为唯一标识
	+ rootUrl 是存放该 App 源码的根路径，这个接口将会根据 AppId 从指定的目录获取 App

2. 利用 QVNApp 打开指定页面的接口

	```objc
	// QVNApp.h
	- (void)startApp:(UIViewController *)parentCtl pageUrl:(NSString *)pageUrl animation:(BOOL)animation;
	```
	
	获取到 QVNApp 后，调用该方法，可以 Push 一个 VN 页面
	
	+ parentCtl: App 内使用 vn.navigateTo 等进行跳转时，需要外部传入一个跳转的 Controller 作为起点，同时加载的 VN 页面也会在该 Controller 上 Push 进来
	+ pageUrl: 页面 URL，必须是 "vn://" 开头的绝对路径，注意与单纯的单页面模式的区别
	+ animation: 是否需要 Push 动画，默认需要

### 3. 使用范例

App 的 VN 代码存放在 Bundle 中，以下图为例，AppId 为 54，info.json 存放有 App 相关的信息，比如AppId、各个页面的配置信息（横竖屏模式、状态栏样式等）等等

```
MyApp.app
└── 54
    ├── index
    │   ├── index.page
    │   └── test.page
    └── info.json
```

```objc
NSString *appId = @"54";
NSString *pageUrl = @"vn://index/index"
NSString *rootUrl = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:appId];
// 获取 App
QVNApp *app = [[QVNVideoNative sharedInstance] getVNApp:appId rootUrl:rootUrl];
// 启动 App 中的某个页面
[app startApp:self pageUrl:pageUrl animation:YES];
```

如果要获取 test.page，则 pageUrl 改为 @"vn://index/test"

## App 的单页面模式
### 1. 简介

该模式相比单纯的单页面模式，多了 App 的概念，因此具有 vn.app、vn.navigate、vn.window 的功能；该页面是 QVNApp 模式的一个扩展功能，比如某个业务需要使用 QVNApp，但是它的跳转入口可能是来自一个 Native 页面上的一个 UIView，这个 UIView 业务需要它也是 VN 开发，那么此时这个 UIView 就可以使用这种模式获取

总的来说，这种模式的使用场景较小

涉及的关键类： QVNApp、QVNPage

### 2. 接口
```objc
// QVNApp.h
- (void)acquirePage:(UIViewController *)parentCtl
            pageUrl:(NSString *)pageUrl
       pageInfoData:(IVNPageInfoData *)pageInfoData
           callback:(IVNAcquirePageCallback)callback;
```

获取到 QVNApp 之后，我们可以通过该方法加载一个单页面

+ pageUrl: 页面 URL，必须是 "vn://" 开头的绝对路径，注意与单纯的单页面模式的区别
+ parentCtl: App 内使用 vn.navigateTo 等进行跳转时，需要外部传入一个跳转的 Controller 作为起点，同时加载的 VN 页面也会在该 Controller 上 Push 进来
+ pageInfoData: 外部已经解析好的页面数据，会优先使用页面数据，如果为空再从根据页面 URL 读取
+ callback: 加载单页面成功的回调


如果业务不需要与 QVNApp 打交道，QVNVideoNative 也提供了一个便捷方法，屏蔽获取 QVNApp 的细节

```objc
// QVNVideoNative.h
- (void)loadAppPage:(NSString *)appId
            rootUrl:(NSString *)rootUrl
            pageUrl:(NSString *)pageUrl
          parentCtl:(UIViewController *)parentCtl
           callback:(IVNLoadPageCallback)callback;
```

### 3. 使用范例
```objc
NSString *appId = @"54";
NSString *pageUrl = @"vn://index/index"
NSString *rootUrl = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:appId];
[[QVNVideoNative sharedInstance] loadAppPage:appId
                                     rootUrl:rootUrl
                                     pageUrl:pageUrl
                                   parentCtl:self
                                    callback:^(QVNPage *page, NSString *appId, NSString *rootUrl, 
                                    NSString *pageUrl, QVNErrorCode errorCode) {
                                        self.page = page;
                                        [self.containerView addSubview:[self.page getView]];
                                    }];
```


## 横向比较
| 模式\类别 | VN 框架的角色    | 特点             | 主要缺点                 | 相关类       | 使用频率 |
| ------------ | --------------------- | ------------------ | ---------------------------- | --------------- | -------- |
| 单纯的单页面 | 渲染并生成 UIView | 使用灵活       | 无法使用 vn.app、vn.navigate、vn.window | QVNPage         | 较多   |
| App          | Push VN 的 Controller | 业务集中       | 侵入性较强              | QVNApp          | 较多   |
| App的单页面 | 以上均可          | App 模式的扩展功能 | 混合后页面关系较复杂 | QVNApp、QVNPage | 相对较少 |