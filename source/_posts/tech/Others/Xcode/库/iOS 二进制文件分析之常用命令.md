## otool
otool 命令全称 object file displaying tool，是针对目标文件的展示工具，可以帮助我们发现应用中使用了哪些系统库，以及调用了哪些对象的方法和属性。

```sh
otool -L path // 查看可执行程序都链接了那些库

otool -L path | grep "xxx" // 筛选是否链接了 xxx 库

otool -ov path // 输出 Object-C 类结构以及定义的方法
```

## lipo
处理架构相关

```sh
lipo -info XXX // 查看静态库所支持的架构 armv7 x86_64 arm64

lipo -remove armv7 origin_xxx.a -output op_xxx.a // 删除静态库包括的 armv7 架构

lipo -thin arm64 origin_xxx.a -output op_xxx.a // 拆分静态库，只保留 arm64 CPU 架构

lipo -create 模拟器架构.a 真机架构.a -output 目标通用架构.a // 合并成通用架构
```

## nm
nm 命令的作用是显示符号表

```sh
nm path // 得到 Mach-O 中的程序符号表

nm -nm path // 目标文件的所有符号

nm -u path //Display only undefined symbols.
```

如查看 JSCore 中是否包含 stepInto 方法

```sh
nm ~/Library/Developer/Xcode/iOS\ DeviceSupport/15.2.1\ \(19C63\)\ arm64e/Symbols/System/Library/Frameworks/JavaScriptCore.framework/JavaScriptCore | grep stepInto
```

## strings
搜索二进制文件中是否包含相关字符串，常用的命令为：

```sh
strings xxx.a | grep "xxx"
```