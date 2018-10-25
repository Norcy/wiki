# 一、调试技巧
## 1.1 自动复制跳转链接
### 场景1
开发同学和测试同学回家了，群里抛出了一个问题，外部拉起腾讯视频某个页面后又出现 bug 啦！！！怎么办，手上又没有 Xcode，怎么快速拿到某次跳转的链接

一个普通的解决方法就是发日志给开发同学定位，这种情况感觉大题小做了，有时候就只是想简单的看看跳转链接长什么样；另一个解决方法是自己到彩蛋里面的日志搜索 "open url"，也是操作起来比较麻烦

### 场景2
开发同学上班了，有 Xcode 可以调试了，想要模拟某个跳转，怎么办？

这种情况一般大家都会到代码里打个断点，查看跳转的 URL，再复制到 Mac 的剪切板，如果是模拟器，就再复制到模拟器里打开；如果是真机调试可能还需要通过微信/RTX 去中转

### 解决方法
针对以上2种场景，彩蛋的功能设置里增加了`自动复制跳转链接`的功能，打开后每一次跳转都会自动复制链接，支持普通的 Action 跳转，外部拉起，3DTouch 和 Push 拉起

为了避免扩大影响范围，只针对 app 本次生命周期有效，重启 app 后该功能默认关闭

## 1.2 Mac 和模拟器之间的纯文本传输
### 场景
有时候我们需要在 Mac 和模拟器之间传输文本（比如上面说到的跳转链接），可以使用以下方法

### 从 MAC 复制到模拟器
1. Mac上：选中文字，【cmd+c】 复制文本到 Mac 剪切板
2. 模拟器：【cmd+v】 复制 Mac 剪切板到模拟器剪切板
3. 模拟器：【cmd+shift+v】 将模拟器剪切板的内容粘贴

这3个步骤连起来，就是一次完整的复制过程，多练几次，受益终生

### 从模拟器复制到 Mac
1. 模拟器：选中文字，【cmd+shift+c】复制文本到模拟器剪切板
2. 模拟器：【cmd+c】复制模拟器剪切板到 Mac 剪切板
3. Mac：【cmd+v】将 Mac 剪切板的内容粘贴

这3个步骤连起来，也是一次完整的复制过程，多练几次，又是受益终生，这个方法跟上面说到的【自动复制跳转链接】搭配使用效果更加，可以非常快速地在 Mac 上得到模拟器某一个跳转链接


## 1.3 Debugger Output
### 场景
调试时断点打了一次 po 信息，继续执行，再次打断点的时候发现上一次的 po 信息已经被 app 的其他日志冲没了，每次遇到这种情况我们难道都要傻傻地翻回去找吗？

### 解决方法
使用 Debugger Output 可以只展示自己的 po 信息
![](http://7xsd8c.com1.z0.glb.clouddn.com/DebuggerOutput.png)

## 1.4 快速切换正式环境和测试环境

+ 很久以前，我们切换环境，要进入好多个页面再退回来；
+ 后来，我们切换环境，可以从个人中心页快速进入；
+ 再后来，春哥加了个自动返回，再也不用点很多次返回了；
+ 现在，双击好莱坞 Tab 头像，可以快速切换正式环境和测试环境。这个很久之前已经说过，有些同学可能还不知道有这个入口，所以再次安利下，习惯之后你会爱上这种感觉

测试同学如果有需要也可以考虑将这些功能放开

# 二、快速Coding
## 2.1 快速设置 view 的背景颜色
### 场景
大家是不是经常写这种代码

```objc
#ifdef DEBUG
    _imageContent.backgroundColor = [UIColor redColor];
    _lineLabel.backgroundColor = [UIColor greenColor];
    _titleLabel.backgroundColor = [UIColor yellowColor];
    _subTitleLabel.backgroundColor = [UIColor grayColor];
    _introLabel.backgroundColor = [UIColor purpleColor];
    self.contentView.backgroundColor = RGBCOLOR(0x2d, 0xff, 0x21);
#endif
```
那有没有感觉写这些代码是对生命的严重浪费

1. 每次调试 UI 都要写这一坨代码
2. 英语学不好，单词不够用的时候还要自己写个RGBCOLOR，小心翼翼地填数字不要超过255
3. 写的时候外面还要记得包一层 DEBUG 宏
4. 开发完毕之后还要把这一坨东西删掉，或者感觉自己写得好辛苦，下次调试还要用，舍不得删，所以就注释掉。食之无味，弃之可惜

### 解决方法
现在你只需要一行代码就可以搞定以上问题

```objc
ColorSubviews(self.contentView);
```

5.0.1好莱坞代码合并之后，将可以使用以下宏定义来快速设置颜色，不需要添加 Debug 保护

```objc
ColorView(view)     // 单个View
ColorViews(views)   // View数组
ColorSubviews(view) // 单个View及其子View
```
# 三、工具
## 3.1 自动访问外网
### 场景
开发网每天都要点击访问外网？烦不烦，累不累

### 解决方法
如果你跟我一样懒，可以使用脚本，出于安全考虑，不放开下载，毕竟是违反规定的，需要的可以找我要，低调使用

1. 设置有线网代理：http://txp-01.tencent.com/proxy_devnet.pac
2. 下载脚本 OutWall.sh 和 OutWall.py，并设置权限为777，存在同一目录
3. 设置开机启动 OutWall.sh（系统偏好设置->用户与群组->登录项 添加OutWall.sh）

注意：

1. 开机后由于要等网络连接，所以脚本会有一定的延迟执行，大概12秒，这段期间不要访问网页，因为频繁的访问（大概一天5次，所以晚上就会失效）会导致提前出现验证码，出现验证码后脚本将在这一天失效。正常情况下，晚上7-8点左右脚本会失效
2. 开机后终端会打开，此时可以退出终端，脚本仍在后台运行
3. 如果想要执行脚本后自动关闭终端窗口，可以这样设置：终端设置 -> 描述文件 -> Shell: 选择当 shell 完全退出后关闭


## 3.2 随时随地发Push
写了一个[Push网页版](http://harray.github.io/js_api/push)，这是恒卓的 Push.app 的网页版

目的只是为了方便开发和测试同学发送 Push，支持下拉选择常用示例，移动端也做了一点设配

目前先挂载在 harray 的 github 上，因为只有这个地址是在我们 app 的域名白名单内

不过搞完才发现外网和 StaffWifi 无法访问 Push 接口，囧

## 3.3 ClangFormat
### 场景
有时候看到一堆代码比较凌乱，影响阅读速度？

### 解决方法
这是个[神器](https://github.com/travisjeffery/ClangFormat-Xcode)，可以快速把代码变成你想要的样子
![](http://7xsd8c.com1.z0.glb.clouddn.com/CodeFormat.gif)