---
date: 2019-08-15
---

## 模块化
模块通常是指编程语言所提供的代码组织机制，利用此机制可将程序拆解为独立且通用的代码单元。所谓模块化主要是解决代码分割、作用域隔离、模块之间的依赖管理以及发布到生产环境时的自动化打包与处理等多个方面

模块化的优点如下：

+ 可维护性。 因为模块是独立的，一个设计良好的模块会让外面的代码对自己的依赖越少越好，这样自己就可以独立去更新和改进
+ 命名空间。 在 JavaScript 里面，如果一个变量在最顶级的函数之外声明，它就直接变成全局可用。因此，常常不小心出现命名冲突的情况。使用模块化开发来封装变量，可以避免污染全局环境
+ 重用代码。 我们有时候会喜欢从之前写过的项目中拷贝代码到新的项目，这没有问题，但是更好的方法是，通过模块引用的方式，来避免重复的代码库

CommonJS 和 AMD 是 JS 中关于模块化的 2 个规范，其中 CommonJS 一般用于服务器，AMD 一般用于浏览器

## CommonJS
在 CommonJS 中，每个 JavaScript 文件就是一个独立的模块上下文（module context），在这个上下文中默认创建的属性都是私有的。也就是说，在一个文件定义的变量、函数和类，都是私有的，对其他文件是不可见的

CommonJS 有四个重要的环境变量为模块化的实现提供支持：module、exports、require、global

> 注意 exports 要与 ES6 的 import & export 区别开

实际使用时，用 module.exports 定义当前模块对外输出的接口（不推荐直接用 exports），用 require 加载模块

```js
// 定义模块 math.js
var basicNum = 0;
function add(a, b) {
  return a + b;
}
module.exports = { // 在这里写上需要向外暴露的函数、变量
  add: add,
  basicNum: basicNum
}
```

```js
// 引用自定义的模块时，参数包含路径，可省略. js
var math = require('./math');
math.add(2, 5);

// 引用核心模块时，不需要带路径
var http = require('http');
http.createService(...).listen(3000);
```

CommonJS 的 require 方法是同步的。在服务端，模块文件都存在本地磁盘，读取非常快，所以这样做不会有问题。但是在浏览器端，数据需要从服务器获取，受限于网络速度，更合理的方案是使用异步加载


注意 module.exports 和 exports 的区别

```js
var basicNum = 0;
function add(a, b) {
  return a + b;
}

// 方案 1：使用 module.exports 一个个导出（正确）
module.exports.add = add
module.exports.basicNum = basicNum

// 方案 2：使用 exports 一个个导出（正确）
exports.add = add
exports.basicNum = basicNum

// 方案 3：使用 module.exports 整体导出（正确）
module.exports = {
  add: add,
  basicNum: basicNum
}

// 方案 4：使用 exports 整体导出（错误）
exports = {
  add: add,
  basicNum: basicNum
}
```

方案 4 是错误的，可以理解为在模块开始前 exports = module.exports，因为赋值之后 exports 失去了 对 module.exports 的引用，成为了一个模块内的局部变量

## AMD
Asynchronous Module Definition 规范，意为 “异步模块定义”

AMD 定义了一套 JavaScript 模块依赖异步加载标准，来解决同步加载的问题。主要包含 define 和 require 两个方法

模块化使得不会污染全局环境，能够清楚地显示依赖关系，允许异步加载模块，也可以根据需要动态加载模块

### define：定义模块
define 方法用于模块的定义

```js
define(
    module_id /* 可选 */, 
    [dependencies] /* 可选 */, 
    factory: Function|Object /* 用来初始化模块或对象的函数 */
);
```

+ 第一个参数 id 参数被省略的时候，我们说这个模块是匿名的
+ 第二个参数 dependencies 参数代表了一组对所定义的模块来说必须的依赖项；如果没有指定 dependencies，那么它的默认值是 ["require", "exports", "module"]
+ 第三个参数 factory，既可以是函数，也可以是对象。如果是对象，此对象应该为模块的输出值。如果是一个函数，它应该只被执行一次；包裹着模块的具体实现，等到依赖加载完成之后，它才会运行。注意 Function 的参数是各个依赖项的输出，顺序与依赖项一一对应，**返回值就是该新定义模块的输出**

当第三个参数是 Object 的时候，如下例，生成了一个拥有 method1、method2 两个方法的模块

```js
define({
    method1: function() {},
    method2: function() {}
});
```

当第三个参数是 Function 的时候，等价的写法如下，这种写法的自由度更高一点，可以在函数体内写一些模块初始化代码

```js
define(function () {
    return {
        method1: function() {},
        method2: function() {}
    };
});
```

一个更具体的例子

```js
define('myModule', ['foo', 'bar'], 
    // 模块定义函数
    // 依赖项（foo 和 bar）被映射为函数的参数
    function (foo, bar) {
        // 返回一个定义了模块导出接口的值
        // （也就是我们想要导出后进行调用的功能）
    
        // 在这里创建模块
        var myModule = {
            doStuff:function(){
                console.log('Yay! Stuff');
            }
        }
 
        return myModule;
    }
);
 
// 另一个例子可以是...
define('myModule', ['jquery', './math.js', 'foo'], function($, math, foo) {
    // $ 是 jquery 模块的输出
    $('body').text('hello world');
});
```

一个使用了简单 CommonJS 转换的模块定义：没有 return 值，输出使用 exports

```
define(function (require, exports, module) {
 var a = require('a'),
     b = require('b');

 exports.action = function () {};
});
```

### require：加载模块
AMD 也采用 require() 语句加载模块，但是不同于 CommonJS，它是异步的，所以多了一个 callback 方法：

`require([module], callback);`

+ 第一个参数是一个数组，里面的成员就是要加载的模块
+ 第二个参数 callback，等加载的模块全部加载成功之后的回调函数

```js
// 假设'foo' 和'bar' 是两个外部模块
// 在本例中，这两个模块被加载后的'exports' 被当做两个参数传递到了回调函数中
// 所以可以像这样来访问他们
require(['foo', 'bar'], function (foo, bar) {
        // 这里写其余的代码
        foo.doSomething();
});
```


require 方法也可以放在 define 内部，比如当 define 的依赖项很多时，参数与模块一一对应的写法非常麻烦

```js
define(
    ['dep1', 'dep2', 'dep3', 'dep4', 'dep5', 'dep6', 'dep7', 'dep8'],
    function(dep1, dep2, dep3, dep4, dep5, dep6, dep7, dep8){
        ...
    }
);
```

为了避免像上面代码那样繁琐的写法，RequireJS 提供一种更简单的写法

```js
define(
    function (require) {
        var dep1 = require('dep1'),
            dep2 = require('dep2'),
            dep3 = require('dep3'),
            dep4 = require('dep4'),
            dep5 = require('dep5'),
            dep6 = require('dep6'),
            dep7 = require('dep7'),
            dep8 = require('dep8');
            ...
    }
);
```
### 动态加载模块

```js
define(function ( require ) {
    var isReady = false, foobar;
 
    require(['foo', 'bar'], function (foo, bar) {
        isReady = true;
        foobar = foo() + bar();
    });
 
    return {
        isReady: isReady,
        foobar: foobar
    };
});
```

上面代码所定义的模块，内部加载了 foo 和 bar 两个模块，在没有加载完成前，isReady 属性值为 false，加载完成后就变成了 true。因此，可以根据 isReady 属性的值，决定下一步的动作

## ES6 模块
### export 和 import
ES6 的模块功能主要由两个命令构成：export 和 import

定义模块 math.js，并输出两个对象

```js
var basicNum = 0;
var add = function (a, b) {
    return a + b;
};
export { basicNum, add };
```

```js
/** 引用模块 **/
import { basicNum, add } from './math';
function test(ele) {
    ele.textContent = add(99 + basicNum);
}
```

### export default
ES6 还提供了 export default 命令，为模块指定默认输出，对应的 import 语句不需要使用大括号

```js
/** export default **/
// 定义输出
export default { basicNum, add };
// 引入
import math from './math';
function test(ele) {
    ele.textContent = math.add(99 + math.basicNum);
}

```

### ES6 与 CommonJS 的模块差异
1. CommonJS 模块输出的是一个值的拷贝，ES6 模块输出的是值的引用

    + CommonJS 一旦输出一个值，模块内部的变化就影响不到这个值

    + ES6 中，JS 引擎编译时遇到模块加载命令 import，就会生成一个只读引用，模块内部的变化会影响已输出的值

2. CommonJS 模块是运行时加载，ES6 模块是编译时输出接口

    + 运行时加载: CommonJS 模块就是对象；即在输入时是先加载整个模块，生成一个对象，然后再从这个对象上面读取方法，这种加载称为 “运行时加载”

    + 编译时加载: ES6 模块不是对象，而是通过 export 命令显式指定输出的代码，import 时采用静态命令的形式。即在 import 时可以指定加载某个输出值，而不是加载整个模块，这种加载称为 “编译时加载”

### ES6 常见的导出导入语法
```js
// 例子 1：使用 defalut
export default defaultName; // 注意导出的 default 只能有一个
import defaultName from './modules.js';

// 例子：2 import default 时可以随意指定命名
export default defaultName;
import myName from './modules.js';

// 例子：3 不使用 default
export {export1, export2}
import {export1, export2} from './module.js'

// 例子：4 import 使用别名
export {export1, export2}
import {export1 as ex1, export2 as ex2} from './module.js'

// 例子：5 export 使用别名
export {export1 as ex1, export2 as ex2}
import {ex1, ex2} from './module.js'

// 例子：6 混合使用
export default defaultName
export {export1, exprot2}
import defaultName, {export1, export2} from './module.js'

## 参考文章
+ [《前端模块化：CommonJS,AMD,CMD,ES6》](https://juejin.im/post/5aaa37c8f265da23945f365c)
+ [《Javascript 模块化编程（二）：AMD 规范》](http://www.ruanyifeng.com/blog/2012/10/asynchronous_module_definition.html)
+ [《AMD 规范》](https://zhaoda.net/webpack-handbook/amd.html)
+ [《使用 AMD、CommonJS 及 ES Harmony 编写模块化的 JavaScript》](https://justineo.github.io/singles/writing-modular-js/)
+ [《RequireJS 和 AMD 规范》](https://javascript.ruanyifeng.com/tool/requirejs.html)
+ [《AMD (中文版)》](https://github.com/amdjs/amdjs-api/wiki/AMD-(中文版))
+ [javascript中import和export用法总结
](https://segmentfault.com/a/1190000016417637)