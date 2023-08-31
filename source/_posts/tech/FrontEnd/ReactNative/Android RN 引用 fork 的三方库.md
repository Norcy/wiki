## 背景
Android 的 RN 工程，如果要对某个三方库进行 Native 代码的修改，该三方库不是自己维护的，因此需要 fork 一份到自己的 Github 仓库。此时要如何才能引用到自己修改过后的库呢？

我对 Android 不熟，iOS 的做法是

1. Fork 原仓库，修改后加标签以及修改其 podspec 文件指向新的标签
2. Podfile 中将引用改为 Commit 引用即可（分支引用或发布自己的 podspec 也是可以的）

经过一番搜索，Android 也可以用类似的做法

## 步骤
### 1. Fork 原仓库，修改代码
我这里要修改的是 react-native 仓库

### 2. 修改版本信息
即修改 ReactAndroid/gradle.properties 的 VERSION_NAME 字段

```sh
-VERSION_NAME=0.62.2
+VERSION_NAME=0.62.2.9.1
```

### 3. 修改 source 信息
修改 ReactAnroid/release.gradle 中的信息，执行自己的仓库，类似修改 iOS 的 podspec 文件

其实就是把里面关于原来的 git 链接替换为自己的

```sh
         artifactId(POM_ARTIFACT_ID)
         packaging(POM_PACKAGING)
         description("A framework for building native apps with React")
-        url("https://github.com/facebook/react-native")
+        url("https://github.com/Norcy/react-native")
 
         scm {
-            url("https://github.com/facebook/react-native.git")
-            connection("scm:git:https://github.com/facebook/react-native.git")
-            developerConnection("scm:git:git@github.com:facebook/react-native.git")
+            url("https://github.com/Norcy/react-native.git")
+            connection("scm:git:https://github.com/Norcy/react-native.git")
+            developerConnection("scm:git:git@github.com:Norcy/react-native.git")
         }
 
         licenses {
             license {
                 name("MIT License")
-                url("https://github.com/facebook/react-native/blob/master/LICENSE")
+                url("https://github.com/Norcy/react-native/blob/master/LICENSE")
                 distribution("repo")
             }
         }
 
         developers {
             developer {
-                id("facebook")
-                name("Facebook")
+                id("Norcy")
+                name("Norcy")
             }
         }
     }
```

### 4. 制作 aar、pom 等产物
接下来就是生成 aar 等产物，幸好这个 RN 仓库就已经自带了，只需要在根目录下执行内置脚本即可

```sh
./gradlew  :ReactAndroid:installArchives
```

该命令执行后会在根目录生成一个 android 文件夹，这里面就是产物信息，如下

```sh
.
└── com
    └── facebook
        └── react
            └── react-native
                ├── 0.62.2.9.1
                │   ├── react-native-0.62.2.9.1-javadoc.jar
                │   ├── react-native-0.62.2.9.1-javadoc.jar.md5
                │   ├── react-native-0.62.2.9.1-javadoc.jar.sha1
                │   ├── react-native-0.62.2.9.1-sources.jar
                │   ├── react-native-0.62.2.9.1-sources.jar.md5
                │   ├── react-native-0.62.2.9.1-sources.jar.sha1
                │   ├── react-native-0.62.2.9.1.aar
                │   ├── react-native-0.62.2.9.1.aar.md5
                │   ├── react-native-0.62.2.9.1.aar.sha1
                │   ├── react-native-0.62.2.9.1.pom
                │   ├── react-native-0.62.2.9.1.pom.md5
                │   └── react-native-0.62.2.9.1.pom.sha1
                ├── maven-metadata.xml
                ├── maven-metadata.xml.md5
                └── maven-metadata.xml.sha1
```

这里我遇到脚本执行出错的几个问题

+ 问题 1：找不到 hermes，通过在仓库根目录 yarn 后即可解决
+ 问题 2：仓库根目录 yarn 失败，通过删除 node_modules 和 yarn.lock 重新 yarn 即可
+ 问题 3：找不到 tools.jar 错误，第一步是在 ~/.zshrc 中配置 JAVA_HOME，其中 JAVA_HOME 的 value 是通过执行 `/usr/libexec/java_home` 得到

```sh
export JAVA_HOME="/Library/Internet Plug-Ins/JavaAppletPlugin.plugin/Contents/Home"
export PATH=$JAVA_HOME/bin:$PATH
```

+ 问题 4：配置后依然找不到 tools.jar，最终使用 Android Studio 下载了一个 JDK，把其中的 tools.jar 拷贝到 JAVA_HOME 的 bin 文件夹下解决

### 5. 配置远程 maven 仓库
这一步类似 iOS 的 podspec 的 source 仓库

1. 新创建一个 Github 仓库
2. 新建一个 repository 文件夹
3. 将上一个步骤的产物信息拖到该文件夹，提交 Git 即可
4. 由此计算最终的 maven 链接，以我的仓库为例 https://github.com/Norcy/maven，最终输出的链接为 https://raw.githubusercontent.com/Norcy/maven/master/repository/，其中 master 是分支名

### 6. 配置新的 maven 源
这一步类似修改 iOS 的 Podfile

现在要切到我们的项目工程

在 android/app/build.gradle 中修改其中的版本引用，类似修改 Podfile 中的版本

```sh
-    implementation "com.facebook.react:react-native:+"  // From node_modules
+    implementation "com.facebook.react:react-native:0.62.2.9.1"  // From node_modules
```

在 android/build.gradle 中修改修改其中的 maven 链接，类似修改 Podfile 中的 source。同时这里也有一个版本号需要一起修改

```sh
allprojects {
     repositories {
+        maven { url 'https://raw.githubusercontent.com/Norcy/maven/master/repository/' }
         jcenter()
         mavenLocal()
         maven {

allprojects {
     configurations.all {
         resolutionStrategy {
             // Remove this override in 0.65+, as a proper fix is included in react-native itself.
-            force "com.facebook.react:react-native:" + REACT_NATIVE_VERSION
+            force "com.facebook.react:react-native:" + '0.62.2.9.1'//REACT_NATIVE_VERSION
             force 'androidx.core:core-ktx:1.6.0'
         }
     }
```

全部完成之后，sync 一下 gradle 即可



## 参考
+ [例子，不过用的是 maven 而不是 gradle 管理](https://github.com/liuyueyi/maven-repository/tree/master)
+ [如何修改配置](https://juejin.cn/post/6844903608652136461)
+ [例子](https://blog.csdn.net/kingwjh/article/details/108196811)