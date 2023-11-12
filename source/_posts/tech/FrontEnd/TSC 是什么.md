## 背景
RN 项目中可以使用 `yarn tsc` 来进行 TS 文件的检查。什么是 tsc 呢？

tsc 是 TypeScript 编译器，用于将 TypeScript 代码转换为 JavaScript 代码。它帮助开发者在编译阶段发现错误，，如类型不匹配、未定义的变量等，以提高代码质量和可维护性

## tsc 的安装
```sh
yarn global add typescript
```

### tsc 的基础使用
安装 tsc ，使用以下命令编译 TypeScript 文件，生成对应的 JavaScript 文件

```
tsc your-file.ts
```

## tsc 的进阶使用
tsc 还支持其他选项和配置，可以通过 `tsc --help` 查看。这些选项可以在配置文件中指定

tsc 的配置文件是一个名为 `tsconfig.json` 的文件，它用于配置 TypeScript 编译器的行为。通过配置文件，你可以指定编译器的选项、编译输出的目录结构以及其他一些配置。

在项目的根目录下创建一个名为 `tsconfig.json` 的文件，并在其中指定你想要的配置选项。以下是一个简单的 `tsconfig.json` 示例：

```json
{
  "compilerOptions": {
    "allowJs": true,
    "allowSyntheticDefaultImports": true,
    "esModuleInterop": true,
    "isolatedModules": true,
    "jsx": "react",
    "lib": ["es6"],
    "moduleResolution": "node",
    "noEmit": true,
    "strict": true,
    "target": "esnext",
    "resolveJsonModule": true,
    "skipLibCheck": true,
    "allowUnusedLabels": true,
    "baseUrl": ".",
    "paths": {"@common/*": ["./common/*"]}
  },
  "exclude": [
    "node_modules",
    "babel.config.js",
    "metro.config.js",
    "jest.config.js",
  ]
}
```

### `allowJs`
允许编译器编译 JavaScript 文件（.js）。

当 `allowJs` 设置为 `true` 时，编译器将会编译项目中的 JavaScript 文件，而不仅仅是 TypeScript 文件。

这意味着你可以在一个 TypeScript 项目中混合使用 JavaScript 和 TypeScript 文件。这个选项的主要作用是允许你逐步迁移现有的 JavaScript 代码到 TypeScript，而无需一次性将所有文件都转换为 TypeScript。你可以先将项目中的 JavaScript 文件保留下来，并在需要的时候逐个进行类型检查和适当的类型注解。

需要注意的是，当 `allowJs` 设置为 `true` 时，编译器将尝试对 JavaScript 文件进行类型检查，但由于 JavaScript 是一种动态类型语言，编译器无法推断出变量的具体类型。

因此，在 JavaScript 文件中，你可能需要手动添加类型注解来帮助编译器进行类型检查。

### `allowSyntheticDefaultImports`
允许在没有默认导出的模块中使用默认导入。

在 JavaScript 中，一个模块可以通过默认导出（default export）或命名导出（named export）的方式暴露其功能。默认导出是指模块中的主要导出，而命名导出是指通过具体名称导出的功能。

然而，在某些情况下，某个模块可能没有默认导出，而只有命名导出。当你尝试使用默认导入语法导入这样的模块时，TypeScript 编译器会发出一个错误，因为它默认情况下假设每个模块都有一个默认导出。

通过将 `allowSyntheticDefaultImports` 设置为 `true`，你告诉编译器允许在没有默认导出的模块中使用默认导入语法。编译器会在背后生成额外的代码来模拟默认导入的行为。

这个选项的主要作用是提供更好的互操作性，允许你在使用第三方库或模块时更自由地使用默认导入语法，而不需要担心是否存在默认导出

### `esModuleInterop`
允许在导入模块时使用 CommonJS 或 AMD 的默认导出语法。

在 JavaScript 中，有两种主要的模块系统：CommonJS 和 AMD。它们在导出和导入模块时使用不同的语法。当 `esModuleInterop` 设置为 `true` 时，TypeScript 编译器会生成额外的代码来兼容使用 CommonJS 或 AMD 的默认导出语法的模块。这样，你可以在 TypeScript 代码中使用更简洁的默认导入语法，而不需要手动进行转换。

具体来说，当你使用 `import` 语句导入一个模块时，如果该模块使用 CommonJS 或 AMD 的默认导出语法，编译器会自动进行转换，以确保导入的模块能够正确地被使用。它简化了在 TypeScript 代码中导入模块的语法，使代码更加清晰和易读。

### `isolatedModules`
将每个文件作为单独的模块进行编译，而不是将它们合并到一个文件中。

为 `true` 时，编译器会将每个文件视为独立的模块，并且会在每个文件中执行严格的模块化检查。这意味着在一个文件中无法使用另一个文件中定义的变量、函数或类，除非通过导入模块的方式进行访问。

这个选项的主要目的是帮助开发者遵循模块化的最佳实践，确保代码的可维护性和可重用性。通过将每个文件作为独立的模块进行编译，可以减少全局命名空间的污染，强制使用显式的导入和导出语法，以及更好地检测潜在的命名冲突和依赖关系问题。

需要注意的是，当 `isolatedModules` 设置为 `true` 时，你需要确保每个文件都使用了适当的导入和导出语法，以便在文件之间进行正确的模块依赖关系。否则，编译器会发出错误。

### `jsx`
指定 JSX 的转换方式，这里设置为 "react"，表示使用 React 的 JSX 语法。RN 项目都如此设置

### `lib`
指定编译时可用的库文件，这里只包含 "es6"，表示使用 ES6 标准库来进行编译检查。RN 项目都如此设置

lib 选项用于指定编译器可以使用的 JavaScript 标准库的集合。标准库提供了一组预定义的类型定义和运行时功能，供 TypeScript 编译器使用

### `moduleResolution`
指定模块解析策略，这里设置为 "node"，表示使用 Node.js 的模块解析方式。RN 项目都如此设置

### `noEmit`
不生成编译输出文件，只进行类型检查。非常常用


### `strict`
启用所有严格的类型检查选项。无脑选 true

### `target`
编译后的 JavaScript 代码的目标版本，这里设置为 "esnext"，表示生成符合最新 ECMAScript 版本的 JavaScript 代码。

### `resolveJsonModule`
允许导入 JSON 文件作为模块。如果项目中导入了 JSON 文件需要开启这个

### `skipLibCheck`
跳过对声明文件的检查，可以加快编译速度。也就是跳过了三方库的检查

### `allowUnusedLabels`
允许存在未使用的标签

### `baseUrl`
指定模块解析的基础路径，默认为当前目录。与 `paths` 配合使用，实现绝对路径的导入

### `paths`
配置模块的路径映射。与 `baseUrl` 配合使用，实现绝对路径的导入


## 如何防劣化
只要在 package.json 中 hook，在每次 Git 提交前检查 tsc 即可

安装 husky

```sh
yarn add -D husky
```

在 package.json 中配置

```json
"husky": {
  "hooks": {
    "pre-commit": "yarn tsc"
  }
},
```