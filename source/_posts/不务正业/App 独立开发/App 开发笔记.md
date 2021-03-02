## 异常上报
使用 Sentry

需要根据[指引](https://docs.sentry.io/platforms/react-native/advanced-setup/)对 Xcode 工程进行配置，除此之外，还针对 Debug 模式不上传 Sentry，防止浪费时间和资源

生成 sourcemap 文件

```sh
export NODE_BINARY=node

if [ "$CONFIGURATION" == "Debug" ]
then
    echo "Norcy_Debug"
    ../node_modules/react-native/scripts/react-native-xcode.sh
else
    echo "Norcy_Release"
    export EXTRA_PACKAGER_ARGS="--sourcemap-output $DERIVED_FILE_DIR/main.jsbundle.map"
    export SENTRY_PROPERTIES=../sentry.properties

    ../node_modules/@sentry/cli/bin/sentry-cli react-native xcode \
      ../node_modules/react-native/scripts/react-native-xcode.sh
fi
```

上传 Debug 符号到 Sentry 平台

```sh
# Type a script or drag a script file from your workspace to insert its path.

if [ "$CONFIGURATION" == "Debug" ]
then
    echo "Norcy_Debug"
else
    echo "Norcy_Release"
    export SENTRY_PROPERTIES=../sentry.properties
    ../node_modules/@sentry/cli/bin/sentry-cli upload-dif "$DWARF_DSYM_FOLDER_PATH"
fi
```

## 用户反馈
[兔小槽](https://txc.qq.com/)


## 日活统计
[友盟](https://mobile.umeng.com/platform/5faa8b291c520d3073a536fc/reports/trend_summary)

## 隐私协议
[App-Privacy-Policy-Generator](https://app-privacy-policy-generator.firebaseapp.com/)

## AppStore 截图
[previewed](https://previewed.app/mockups/screenshots/appstore/iphone-panorama)

## App Icon 生成
[蒲公英](https://www.pgyer.com/tools/icon)

## icon 素材
+ [阿里巴巴](https://www.iconfont.cn/home/index)
+ [react-native-vector-icons](https://oblador.github.io/react-native-vector-icons/)

> react-native-vector-icons 优点是占用体积小，缺点是图标有限

最新的 Xcode 安装 react-native-vector-icons 后，每次 pod install 之后都会编译出错，原因是资源重复导入，因此写了以下脚本进行删除

RN 的根目录运行即可

```sh
cd ios
pod _1.9.3_ install
sed -i '' 's/.*.ttf\",//g' Paxxword.xcodeproj/project.pbxproj
```

## 配色创意
+ [颜色表](http://www.5tu.cn/colors/yansebiao.html)
+ 色采 App



## 热更新
+ [react-native-code-push](https://github.com/microsoft/react-native-code-push)

```sh
# 安装
yarn global add appcenter-cli
# 登录
appcenter login
# 列出 app
appcenter apps show
# 设置当前 App
appcenter apps set-current Nx/Paxxword
```

常用命令

```sh
# 新增模式
appcenter codepush deployment add Release
appcenter codepush deployment add Beta
# 列出生效的 JS 包
appcenter codepush deployment list
# 列出生效的 JS 包的 Key
appcenter codepush deployment list -k
# 发布
appcenter codepush release-react -d Release -t 1.0.0 --description "Test Release"
# 发布历史
appcenter codepush deployment history Release
```

客户端需要修改的地方

1. 代码只要改 AppDelegate 就行
2. 新增 Beta 模式
3. 新增 CODEPUSH_KEY 编译选项，Beta 和 Release 的 Key 不同
4. Info.plist 新增 CodePushDeploymentKey，值为 $(CODEPUSH_KEY)