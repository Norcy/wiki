## USB 调试

1. 连接 USB 后运行 `adb reverse tcp:8081 tcp:8081`
2. 执行 `yarn run android` 安装 APK
3. console 按下 R 即可

> 不需要通过摇一摇去设置 IP

## 编译 APK

1. 参考 https://reactnative.cn/docs/signed-apk-android 完成签名配置
2. 点击 Android Studio 的 Build -> Generate Signed Bundle/APK -> APK -> 根据你的签名配置填好即可
3. Android Studio 的弹框中会提示 APK 地址

## 安装 APK 到手机

USB 连接手机且打开手机的 USB 安装开关前提下

- 发布时：执行 `adb install xxx.apk` 即可
- 调试时：执行 `npx react-native run-android`

## 运行 Release 包

点击 Build 控制台左边的 Build Variants，将 :app 的编译模式从 debug 改为 release，再运行即可

## 修改安卓版本号

更改 `app/src/main/AndroidManifest.xml` 中的 versionName 字段

## 通过 ADB 打开手机网址

```sh
adb shell am start -a android.intent.action.VIEW -d https://www.qq.com
```

## 命令行打包 apk 失败
参考 [官方文档](https://reactnative.cn/docs/0.72/signed-apk-android)，执行 `./gradlew assembleRelease` 时报错

```sh
:compileReleaseKotlin FAILED
Kotlin could not find the required JDK tools in the Java installation. Make sure Kotlin compilation is running on a JDK, not JRE
```

修改 `~/.zshrc` 的 `JAVA_HOME` 为 Android Studio 即可

```sh
export JAVA_HOME=/Applications/"Android Studio.app"/Contents/jbr/Contents/Home
# export JAVA_HOME="/Library/Internet Plug-Ins/JavaAppletPlugin.plugin/Contents/Home"
export PATH=$JAVA_HOME/bin:$PATH
```

## 无线调试
1. 先插线，确保手机上允许 USB 调试，然后可以断开线
2. 打开手机的无线调试，记住该页面展示的 IP 和 Port
3. `adb connect IP:Port`
4. `yarn run android`

如果提示 `adb pair` 找不到，可能是 adb 版本过低导致，可通过 `adb --version` 确认版本，30 以上的版本即可使用

如果版本太低，需要更新 adb，打开 Android Studio 的设置，【Android SDK】-> 【SDK Tools】->【Android SDK Platform-Tools】勾选后确认即可更新

详见 [Android 无线调试教程](https://www.cnblogs.com/qianguyihao/p/3824988.html)