## 背景
1. 双端均能使用 xxx:// 直接打开 App
2. 双端均能使用 https://yyy.com/zzz 直接打开 App

## Android
### 使用 scheme 打开打开 App

AndroidManifest.xml 中新增代码如下

```xml
<intent-filter>
    <action android:name="android.intent.action.VIEW" />
    <category android:name="android.intent.category.DEFAULT" />
    <category android:name="android.intent.category.BROWSABLE" />
    <data android:scheme="xxx" />
</intent-filter>
```

### 使用 http 地址打开 App
全程使用 Android Studio 的 App Links Assistant 即可配置完成

AndroidManifest.xml 中新增代码如下

```xml
<intent-filter android:autoVerify="true">
    <action android:name="android.intent.action.VIEW" />
    <category android:name="android.intent.category.DEFAULT" />
    <category android:name="android.intent.category.BROWSABLE" />
    <data android:scheme="https" android:host="yyy.com" android:pathPrefix="/zzz"/>
</intent-filter>
```

将生成的 assetlinks.json 复制到网站根目录

```sh
# ssh 登录后
mkdir /var/www/html/.well-known
cp assetlinks.json /var/www/html/.well-known
```



## iOS
### 使用 scheme 打开打开 App
Info.plist -> URL types 新增一条记录

URL identifier 随便写，URLSchemes 的第一个元素填写 xxx

### 使用 http 地址打开 App
待补充


## 附录
+ [测试用例](https://docs.qq.com/sheet/DSHpXb29STWpYempK?tab=w20187)
+ Android 的 AppLink 配置参考 [Android APP Links 配置](https://bigocto.github.io/2017/12/09/Android-APP-Links-%E9%85%8D%E7%BD%AE/)
+ 安卓测试方法

  1. 生成二维码之后扫一扫
  2. adb shell am start -W -a android.intent.action.VIEW -d "https://yyy.com/zzz?page=VIP" com.your.package
  3. adb shell am start -a android.intent.action.VIEW -c android.intent.category.BROWSABLE -d "https://yyy.com/zzz?page=VIP"
