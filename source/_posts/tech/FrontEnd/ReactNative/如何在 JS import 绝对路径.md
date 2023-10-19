## 背景

引入项目中的模块时，默认使用了相对路径，由于 i18n 需要批量替换 import，而每个文件相对 i18n 模块的位置不一样，那此时就无法完成批量替换

解决方法是使用绝对路径来 import 模块

Before

```js
import i18n from '../common/other/i18n';
```

After

```js
import i18n from '@common/other/i18n';
```

## 方法

1. 安装 babel 插件

```sh
yarn add babel-plugin-module-resolver -D
```

2. babel.config.js 中使用插件，如下的 plugins 信息

```sh
module.exports = {
  presets: ['module:metro-react-native-babel-preset'],
  plugins: [
    [
      'module-resolver',
      {
        extensions: ['.js', '.jsx', 'ts', 'tsx'],
        root: ['.'],
        alias: {
          '@common': './common',
        },
      },
    ],
  ],
};
```

3. 为了能够实现在 VSCode 中点击跳转，还需要配置 tsconfig.json（如果是 js 工程使用 js.config.json）

在 compilerOptions 中新增 baseUrl 和 paths

```json
{
  "compilerOptions": {
    "baseUrl": ".",
    "paths": {"@common/*": ["./common/*"]}
  }
}
```