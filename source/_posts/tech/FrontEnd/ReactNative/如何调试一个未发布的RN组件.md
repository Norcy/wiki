## 背景
当开发一个新的 RN 组件时，发布之前可能需要经过业务场景的测试。如何测试才能更高效呢？

这里涉及两个路径，用以下字符串代替

+ 开发中的组件的路径：`/path/to/my-package`
+ 想引用该组件的工程的路径：`/path/to/my-project`

## 最佳实践：使用 wml
wml 的本质就是监听指定文件的变化，然后及时复制该文件到你指定的地方

所以在我们这个场景，就是利用 wml 把组件的代码自动复制到工程目录下的 `node_modules`

### Step 1. 安装并配置 wml
[wml 官方文档](https://github.com/wix-incubator/wml)

```sh
yarn global add wml # 全局安装 wml
watchman watch `yarn global dir`/node_modules/wml/src # 确保能被 watchman 监听
```

### Step 2. 开始使用 wml
```sh
wml add /path/to/my-package /path/to/my-project/node_modules #指定复制规则
wml start # 开始监听并复制
```

start 之后记得随意修改下组件，这样就能触发自动复制

附录 wml 其他常用方法

```sh
wml ls # 列出当前的复制规则
wml rm [linkId/all]
wml enable [linkId]
wml disable [linkId]
```

### Step 3. 可选：如果是原生组件，你还需要安装原生依赖

跟普通的原生组件类似，你需要安装原生依赖，然后重新安装 App

```sh
npx pod-install # For iOS
```


## 不推荐的方法
### 使用 `npm link` 或 `yarn link`
```sh
cd /path/to/my-package
yarn link
cd /path/to/my-project
yarn link my-package
```

然后你可以在工程中这样使用

```js
const { someMethod } = require('my-awesome-package');
// ... or
import { someMethod } from 'my-awesome-package';
```

最终你会发现这样的错误

```sh
error: Error: Unable to resolve module my-package from /Users/mac/MyReactNativeApp/src/App.tsx: my-package could not be found within the project or in these directories:
node_modules
../../node_modules
```

因为 `yarn link` 的原理就是在 `node_modules` 中创建组件文件夹的软链，而 RN 默认使用的 metro 不支持这种，详细请看 [这篇文章](https://github.com/facebook/metro/issues/1)

### 修改 Metro 配置
修改 metro.config.js，确保组件模块能被解析；同时添加 watchman 监听

```js
const packagePath = '/path/to/my-package';

module.exports = {
    resolver: {
        nodeModulesPaths: [packagePath],
        // rest of metro resolver options...
    },
    watchFolders: [packagePath],
    // rest of metro options...
};
```

然后在你的工程中这样引入

```sh
yarn add /path/to/my-package
```

经测试，这种方法会导致运行报错

## 参考
+ [《Linking Local Packages in React Native the Right Way》](https://medium.com/@alielmajdaoui/linking-local-packages-in-react-native-the-right-way-2ac6587dcfa2)