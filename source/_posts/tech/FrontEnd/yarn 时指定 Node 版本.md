执行 yarn 时可能遇到 Node 版本不匹配

```sh
The engine "node" is incompatible with this module. Expected version "14 || >=16.14". Got "16.13.2"
```

此时我们可以通过 nvm 来安装对应的 Node 版本，并使用

````sh
nvm install 16.14.2
nvm use 16.14.2
node -v
```

如果需要在项目中指定 Node 的版本，让别人 clone 时能遵循这个 Node 的版本限制，可以创建 .nvmrc 文件来强制指定该项目的 Node 版本

```sh
echo v16.14.2 > .nvmrc
```
