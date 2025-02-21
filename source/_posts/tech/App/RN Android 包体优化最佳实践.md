# RN Android 包体优化最佳实践

## ABI 架构优化
如果你的 App 只有真实设备，不使用模拟器，则可以移除 x86 架构，最佳实践如下

android/app/build.gradle 文件中

```groovy
android {
    defaultConfig {
      ndk {
          abiFilters "armeabi-v7a", "arm64-v8a"
      }
    }
    splits {
      abi {
        enable false
        include "arm64-v8a", "armeabi-v7a"//, "x86", "x86_64"
        universalApk true
      }
    }
}
```

1. enable 为 true，则会生成多个架构的 apk，除非你要单独打单架构的包，否则尽量设置为 false
2. universalApk 为 true 时，表示生成通用 APK，该字段不会读取 include 配置，如果要过滤掉 x86 架构，需要设置 ndk.abiFilters 为你要支持的架构