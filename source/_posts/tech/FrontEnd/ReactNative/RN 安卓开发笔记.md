## 设置 IP 地址
1. 连接 USB 后运行 `adb reverse tcp:8081 tcp:8081`
2. console 按下 R 即可

> 不需要通过摇一摇去设置

## 编译 APK
1. 参考 https://reactnative.cn/docs/signed-apk-android 完成签名配置
2. 点击 Android Studio 的 Build -> Generate Signed Bundle/APK -> APK -> 根据你的签名配置填好即可
3. Android Studio 的弹框中会提示 APK 地址

## 安装 APK 到手机
USB 连接手机且打开手机的 USB 安装开关前提下

+ 发布时：执行 `adb install xxx.apk` 即可
+ 调试时：执行 `npx react-native run-android`

## 运行 Release 包
点击 Build 控制台左边的 Build Variants，将 :app 的编译模式从 debug 改为 release，再运行即可


## 