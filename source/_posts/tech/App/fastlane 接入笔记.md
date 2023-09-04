## 流程
1. 安装 fastlane，不详述

2. 初始化 fastlane

```sh
cd ios
fastlane init
# 选择 3. Automate App Store distribution
# 输入 Apple 账号和密码
# 可以暂时选择不管理 meta.json，后续再配置中再打开 meta.json
# 根据最下面的文件进行修改即可
```

## 报错
```sh
Unable to upload archive. Failed to get authorization for username
```

解决方法：
1. 打开 http://appleid.apple.com/
2. 创建一个 App 专用密码并复制
3. 修改 Fastfile，在 before_all 中添加用户名和专属密码的配置，注意写的位置

参考： https://stackoverflow.com/questions/54341690/sign-in-with-the-app-specific-password-you-generated-if-you-forgot-the-app-spec

## 报错
```sh
You can disable IAP checking by setting the `include_in_app_purchases` flag to `false`
```

解决方法：

upload_to_app_store 函数添加 precheck_include_in_app_purchases: false

## 报错
```sh
[altool] 2023-09-04 20:31:52.016 *** Error: The provided entity includes an attribute with a value that has already been used The bundle version must be higher than the previously uploaded version: ‘10’. (ID: ca89416b-aae9-4059-8d53-bc7902b7a7d6) (-19232)
```

解决方法：
新增 increment_build_number(xcodeproj: "iRead.xcodeproj")

## 最终成果
```rb
default_platform(:ios)

platform :ios do
  desc "Setting User and specific password"
  before_all do
    ENV['FASTLANE_USER'] = 'xxx'
    ENV['FASTLANE_APPLE_APPLICATION_SPECIFIC_PASSWORD'] = 'xxx'
  end

  desc "Push a new release build to the App Store"
  lane :release do
    increment_build_number(xcodeproj: "iRead.xcodeproj")
    increment_version_number(bump_type: "minor", xcodeproj: "iRead.xcodeproj")
    build_app(workspace: "iRead.xcworkspace", scheme: "iRead")
    upload_to_app_store(skip_metadata: false, skip_screenshots: true, precheck_include_in_app_purchases: false)
  end
end
```