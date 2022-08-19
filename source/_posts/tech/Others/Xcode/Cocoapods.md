---
title: Cocoapods
---

## VideoNative 项目与 Cocoapods
### 三个仓库
1. VideoNative/VideoNative 日常开发用的仓库（开发仓库）
2. VideoNative/VideoNativeFramework 稳定版本的仓库，存放框架文件，用于发布（发布仓库）
3. VideoNative/VideoNativeSpecs 远程的私人 Cocoapods 源，存放  VideoNative.podspec（说明书仓库）

### 发布流程
1. 用开发仓库的最新代码打包一个 Framework，拷贝到 VideoNativeFramework 仓库（本质：发布仓库在本地更新框架）
2. 编辑 VideoNativeFramework 下的 VideoNative.podsepc 文件，更新版本号（或者依赖文件，如果需要的话）（本质：发布仓库在本地更新说明书）
3. 执行 `pod lib lint` 进行本地检查，如果没问题就 Push 到远程（发布仓库在远程更新框架和说明书）
4. 再执行 `pod spec lint` 进行远程检查（校验发布仓库的框架和说明书）
5. 最后执行以下命令，将 VideoNativeFramework 的 VideoNative.podspec 推送到私人 Cocoapods 源（提交说明书到说明书仓库）

    ```ruby
    pod repo push oa-videonative-vnspec VideoNative.podspec --verbose --allow-warnings
    ```
6. 执行 `pod search VideoNative` 检查是否发布新版本成功（校验说明书仓库）

## 其他
+ spec 的全称是 specification，意为“说明书；技术规范”
+ Cocoapods/Specs 的意思是这个是 Cocoapods 公有仓库的说明书合集
+ podspec 就是每个 pod 组件的说明书文件


## `pod lib lint` 和 `pod spec lint` 区别
+ `pod lib lint` 只对本地的 podspec 进行检查，没有联网操作
+ `pod spec lint` 对远程仓库（Spec Repo）的 podspec 进行检查

## podfile.lock 是什么
可以参考这里 [CocoaPods的原理与技巧（二）](https://www.jianshu.com/p/fb202af858fd)

## 优秀文章
+ [我所理解的 CocoaPods](https://juejin.im/post/5b1cfaff6fb9a01e417b6051)
+ [一行命令发布 Pod 框架](https://juejin.im/entry/58df270f61ff4b006b1227c9)（生成 podspec 的工具不错）
