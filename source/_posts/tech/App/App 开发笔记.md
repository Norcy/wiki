## 异常上报
### 新版本的 Sentry 配置
目前新版本是 

```json
"@sentry/cli": "^1.59.0",
"@sentry/react-native": "^3.4.3",
```

最新的 Sentry 只需要运行一下命令

```sh
npx @sentry/wizard -i reactNative -p ios
npx pod-install
```

为了 Debug 模式不上传 Sentry，防止浪费时间和资源，可以对 `Bundle React Native code and images` 任务改造为

```sh
export NODE_BINARY=node
if [ "$CONFIGURATION" == "Debug" ]
then
    echo "Norcy_Debug1"
    ../node_modules/react-native/scripts/react-native-xcode.sh
else
    echo "Norcy_Release1"
    export EXTRA_PACKAGER_ARGS="--sourcemap-output $DERIVED_FILE_DIR/main.jsbundle.map"
    export SENTRY_PROPERTIES=sentry.properties

    ../node_modules/@sentry/cli/bin/sentry-cli react-native xcode \
      ../node_modules/react-native/scripts/react-native-xcode.sh
fi
```

对 `Upload Debug Symbols to Sentry` 勾选 `Fot install builds only` 即可

### 老版本的 Sentry 配置
以下是老的配置，不需要再看了

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

### 安装
```sh
# 安装
yarn global add appcenter-cli
# 登录
appcenter login
```

客户端需要修改的地方

### App 新接入
1. 到 https://appcenter.ms/apps 新增一个 App
2. 命令行设置当前 App

    ```sh
    # 列出所有 app，确认新增的 App 生效
    appcenter apps list
    # 设置/切换当前 App
    appcenter apps set-current Nx/iRead
    # 列出当前 app，确认切换生效
    appcenter apps show
    # 新增 Release 模式
    appcenter codepush deployment add Release
    # 获取 CODEPUSH_KEY
    appcenter codepush deployment list -k
    ```

3. 修改 AppDelegate，详情请参考[官方文档](https://github.com/microsoft/react-native-code-push/blob/master/docs/setup-ios.md)

4. Info.plist 新增 CodePushDeploymentKey，值为第二步获取到的 CODEPUSH_KEY


### 热更新步骤
1. 确认当前生效的 App

    ```sh
    # 列出所有 app，确认新增的 App 生效
    appcenter apps list
    # 如果 App 不对则需要切换 App
    appcenter apps set-current YourAppName
    ```

2. 确认当前生效的 JS 版本

    ```sh
    # 列出生效的 JS 包
    appcenter codepush deployment list
    ```

3. 发布新版本
    
    ```sh
    # 发布
    appcenter codepush release-react -d Release -t "1.0.0" --description "Message"
    ```

    > **注意这里的版本号是 Native App 的版本号，每次热修复的时候都需要注意这个值**

    > 注意如果要覆盖 >=2.0.0 < 3.0.0 的版本，请不要使用 "~2.0.0"，覆盖不到 2.1.0，应该使用 ">=2.0.0"

    > 你可以使用 -m 表示 Mandatory，强制更新，表示越快越好

### 附录
```sh
# 列出生效的 JS 包的 Key（不常用）
appcenter codepush deployment list -k
# 发布历史
appcenter codepush deployment history Release
```

[包发布后台](https://appcenter.ms/users/Nx/apps/iRead/distribute/code-push/Release)


## Universal Link
[Universal Link 配置教程](https://www.cnblogs.com/itlover2013/p/14873153.html)


## 发布 AppStore
### Archive 失败
正常编译真机能通过，但是 Archive 提示 `library not found for -lBVLinearGradient`，原因是 podfile 写的是最低系统版本是 iOS13，而 Xcode 中是 iOS10，而 Archive 是全部平台编译，所以就会导致编译 iOS10 的时候找不到对应的库。修改方法就是让这两处写法保持一致即可

### 每次 Archive 记得先修改 Build 号