假设现在有一个 helloWorld 工程，已经在其 package.json 的 dependencies 中声明了 packageA，有两个插件 plugin1 和 plugin2 他们也依赖 packageA，如果在插件中使用 dependencies 而不是 peerDependencies 来声明 packageA，那么 `npm install` 安装完 plugin1 和 plugin2 之后的依赖图是这样的：

```sh
.
├── helloWorld
│   └── node_modules
│       ├── packageA
│       ├── plugin1
│       │   └── nodule_modules
│       │       └── packageA
│       └── plugin2
│       │   └── nodule_modules
│       │       └── packageA
```

从上面的依赖图可以看出，helloWorld 本身已经安装了一次 packageA，但是因为因为在
plugin1 和 plugin2 中的 dependencies 也声明了 packageA，所以最后 packageA 会被安装三次，有两次安装是冗余的。

而 peerDependency 就可以避免类似的核心依赖库被重复下载的问题。

如果在 plugin1 和 plugin2 的 package.json 中使用 peerDependency 来声明核心依赖库，例如：

plugin1/package.json

```json
{
  "peerDependencies": {
    "packageA": "1.0.1"
  }
}
```

plugin2/package.json

```json
{
  "peerDependencies": {
    "packageA": "1.0.1"
  }
}
```

在主系统中声明一下 packageA:

helloWorld/package.json

```json
{
  "dependencies": {
    "packageA": "1.0.1"
  }
}
```

此时在主系统中执行 `npm install` 生成的依赖图就是这样的：

```sh
.
├── helloWorld
│   └── node_modules
│       ├── packageA
│       ├── plugin1
│       └── plugin2
```

可以看到这时候生成的依赖图是扁平的，packageA 也只会被安装一次。

因此我们总结下在插件使用 dependencies 声明依赖库的特点：

+ 如果用户显式依赖了核心库，则可以忽略各插件的 peerDependency 声明；
+ 如果用户没有显式依赖核心库，则按照插件 peerDependencies 中声明的版本将库安装到项目根目录中；
+ 当用户依赖的版本、各插件依赖的版本之间不相互兼容，会报错让用户自行修复；