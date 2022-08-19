npx 主要有两个作用

## 作用 1：调用项目安装的模块
如果要在命令行中调用 `node_modules` 中的模块，使用 npx 之前，你需要

```sh
# 项目的根目录下执行
$ node-modules/.bin/mocha --version
```

使用 npx 之后，你只需

```sh
npx mocha --version
```

其原理是运行的时候，会到 node_modules/.bin 路径和环境变量 $PATH 里面，检查命令是否存在。

由于 npx 会检查环境变量$PATH，所以系统命令也可以调用

如

```sh
npx ls
```

> 注意，Bash 内置的命令不在 $PATH 中，所以 npx cd 无效

## 作用 2：避免全局安装模块
如开发 react-native 时使用的 pod-install

```sh
npx pod-install
```

虽然我本机没有安装 pod-install 模块，但是使用 npx 可以直接运行，其原理是先把该模块下载到临时目录，使用以后再删除

## 小技巧：临时切换 node 版本
```sh
npx node@0.12.8 -v
```

## 小技巧：执行 Github 源码
```sh
# 执行 Gist 代码
$ npx https://gist.github.com/zkat/4bc19503fe9e9309e2bfaa2c58074d32

# 执行仓库代码
$ npx github:piuccio/cowsay hello
```

## 参考
+ [阮一峰老师的 npx 使用教程](https://www.ruanyifeng.com/blog/2019/02/npx.html)