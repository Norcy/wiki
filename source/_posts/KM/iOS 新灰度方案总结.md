# 原有的灰度方案
我们原来是在 App 内打开下载链接，直接进行下载，替换 App

```objc
[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"itms-services://?action=download-manifest&url=https://mac.video.qq.com/iphone/manifest.plist"]];
```

但是该方案到 iOS 9 后就莫名失效了，表现是 App 被杀死，但是一直处于等待中，没有下载进度，最后提示下载失败

导致的结果就是我们就无法对 iOS 9 后的用户进行灰度

# 新方案
## 原理
因为是在 App 中打开下载链接出了问题，所以我们换另一种方法，在 App 外面去打开下载链接

之前想了一个解决方法就是，先引导用户打开一个 Web 页面，在 Web 页面再引导用户进行下载安装。

## 可行性验证

1. 如果用户没装腾讯视频，即全新安装，使用这种方法是没问题的
2. 如果用户已经安装了腾讯视频，即覆盖安装
    + 如果他安装的是经过企业证书重签的，那么覆盖安装也是没问题
    + 如果他安装的是 appstore 上下载的，那么覆盖安装失败

为此，针对这种情况，我们的做法是在 H5 上提示用户将原有的腾讯视频删除，再进行安装。这种方法有利也有弊，后续再看看有无更佳的方法

## 准备

为此我们先需要准备3个文件

1. ipa 文件
2. plist 文件
3. H5 页面

为什么呢？这个时候要先扯扯 iPhone 通过 url 安装 app 的原理

iPhone 系统会识别类似以下格式的链接，得到 plist 文件，再从 plist 文件里得到 ipa 的地址及其相关信息，再进行安装

```
itms-services://?action=download-manifest&url=xxx.plist
```

## ipa 文件
ipa 包没什么好说的

## plist 文件
plist 文件大概长这样

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>items</key>
	<array>
		<dict>
			<key>assets</key>
			<array>
				<dict>
					<key>kind</key>
					<string>software-package</string>
					<key>url</key>
					<string>http://10.70.107.98/123.ipa</string>
				</dict>
			</array>
			<key>metadata</key>
			<dict>
				<key>bundle-identifier</key>
				<string>com.tencent.live4iphone</string>
				<key>bundle-version</key>
				<string>14332</string>
				<key>kind</key>
				<string>software</string>
				<key>title</key>
				<string>腾讯视频</string>
			</dict>
		</dict>
	</array>
</dict>
</plist>
```

可以看到 plist 文件包含了 ipa 地址，以及 bundleId，version，应用名称等信息，plist 文件的产生可以借助[这个工具](https://s3-ap-southeast-1.amazonaws.com/problem-arc/Beta-Builder/BetaBuilder.zip)

## H5 页面
那么剩下的问题就是写一个用户可以接受的 H5，用户点击安装按钮之后，执行 JS 进行安装

```js
<script type="text/javascript">
    function install() 
    {
        window.location.assign("itms-services://?action=download-manifest&url=https://mac.video.qq.com/iphone/manifest.plist");
    }
    alert("asd");
</script>
```

[Demo请点我](https://mac.video.qq.com/iphone/index.html)（微信里不能安装，Safari 才行）

# 其他
+ 对于我们 App 不需要改动代码，后台修改下下发的链接即可
+ 一些外网用户可能看到 UI 错乱的页面，原因是 CSS 和 JS 的地址部分地区访问有问题，于是将 CSS 和 JS 各提取成一个文件，放到我们自己的服务器上，将`http://cdn.bootcss.com/bootstrap/3.3.0/css/bootstrap.min.css`改为`https://mac.video.qq.com/iphone/bootstrap.min.css`，JS 同理
+ H5 页面需要适配移动设备
+ plist 的地址一定要 HTTPS，否则无效
+ 微信/QQ 等 app 打开该页面是无法进行安装的，因为安装app所在页面必须要符合苹果itms-services协议，在不符合该协议的页面点击安装会没有反应。部分第三分浏览器支持itms-services协议，可以直接点击安装。