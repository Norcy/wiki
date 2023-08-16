## 其他
1. TypeScript 规定，变量只有赋值后才能使用，否则就会报错

    ```typescript
    let x:number;
    console.log(x) // 报错
    ```
    上面示例中，变量`x`没有赋值就被读取，导致报错。而 JavaScript 允许这种行为，不会报错，没有赋值的变量会返回`undefined`

## 类型声明并不是必需的

如果没有声明，TypeScript 会自己推断类型

```typescript
let foo = 123;
foo = 'hello'; // 报错
```

TypeScript 也可以推断函数的返回值，**正因如此，函数返回值的类型通常是省略不写的**

```typescript
function toString(num:number) {
    return String(num);
}
```

    
## 三种特殊类型

+ any：
    + 定义：任意类型，对此 TS 会关闭类型检查
    + 注意：any 类型不一定等于没写类型，因为没写时 TS 有可能自己推导出类型。
    + 缺点：**any 类型存在污染变量，慎用**
        ```typescript
        let x:any = 'hello';
        let y:number;

        y = x; // 不报错
        y*123 // y 值其实是字符串了，但不报错
        ```
    + 适用场景：重构老项目时可用，其他场景少用
+ unknown：
    + 定义：与 any 类似，所有类型的值都可以赋值给 unknown，但可解决 any 的污染变量问题
        ```typescript
        let x:unknown;

        x = true; // 正确
        x = 42; // 正确
        x = 'Hello World'; // 正确
        ```
    + 注意：unknown 无法赋值给 any/unknown 以外类型，以此解决了 any 的污染变量问题
        ```typescript
        let v:unknown = 123;

        let v1:boolean = v; // 报错
        let v2:number = v; // 报错
        ```
    + 注意：直接调用 unknown 的属性和方法都是不允许的，除非经过类型缩小
        ```typescript
        let v1:unknown = { foo: 123 };
        v1.foo  // 报错

        let v2:unknown = 'hello';
        v2.trim() // 报错

        let s:unknown = 'hello';
        if (typeof s === 'string') {
            s.length; // 正确
        }
        ```
    + 适用场景：**凡是需要设为 any 类型的地方，通常都应该优先考虑设为 unknown 类型**
+ never
    + 定义：空类型。即不可能有值。
    + 注意：对应集合论中的空集，never 类型是所有类型的子集，因此可赋给所有类型
    + 适用场景：**不可能返回值的函数，或永久执行的函数，返回值的类型就可以写为 never**，never 不是 void，前者表示函数没有执行结束，不可能有返回值；后者表示函数正常执行结束，但是不返回值
        ```typescript
        function f():never {
            throw new Error('Error');
        }

        function g():never {
            while(true) {
                console.log('forever')
            }
        }

        let v1:number = f(); // 不报错
        let v2:string = f(); // 不报错
        let v3:boolean = f(); // 不报错
        ```

## 类型系统
JS 中的 8 种类型（不是 TS）

- boolean
- string
- number
- bigint
- symbol
- object
- undefined
- null

注意，**所有类型的名称都是小写字母，首字母大写的`Number`、`String`、`Boolean`等在 JavaScript  语言中都是内置对象，而不是类型名称**

在 JS 中，`typeof`运算符只可能返回八种结果，与上面的类型不同。注意 `null` 的类型是 `object`，以及多了一个 `function` 类型

```javascript
typeof undefined; // "undefined"
typeof true; // "boolean"
typeof 1337; // "number"
typeof "foo"; // "string"
typeof {}; // "object"
typeof null // "object"
typeof parseInt; // "function"
typeof Symbol(); // "symbol"
typeof 127n // "bigint"
```

而在 TS 中，typeof 返回的是该值的 TS 类型

```typescript
const a = { x: 0 };
type A = typeof a   // 无法打印 A，因为 A 编译时会被删除，此时 A 的值应该是 {x: number}，这里是 TS 的 typeof
console.log(typeof a)   // "object"，这里是 JS 的 typeof
```

**同一段代码可能存在两种`typeof`运算符，一种用在值相关的 JavaScript 代码部分，另一种用在类型相关的 TypeScript 代码部分**

```typescript
let a = 1;
let b:typeof a;

if (typeof a === 'number') {
  b = a;
}
```

上面示例中，用到了两个`typeof`，第一个是类型运算，第二个是值运算。它们是不一样的，不要混淆。

JavaScript 的 typeof 遵守 JavaScript 规则，TypeScript 的 typeof 遵守 TypeScript 规则。它们的一个重要区别在于，编译后，前者会保留，后者会被全部删除。

上例的代码编译结果如下。

```typescript
let a = 1;
let b;
if (typeof a === 'number') {
    b = a;
}
```

上面示例中，只保留了原始代码的第二个 typeof，删除了第一个 typeof


## undefined 与 null

另外，`undefined` 和 `null` 既可以作为值，也可以作为类型，取决于在哪里使用它们

1. `undefined` 类型可包含一种值 `undefined`（不像 `boolean`，包含 true 和 false）。前者是类型，后者是值，需要区分

```typescript
let x:undefined = undefined;
```

2. null 类型可包含一种值 `null`。前者是类型，后者是值，需要区分

```typescript
const x:null = null;
```

3. 类型未声明，值为 `undefined`或`null`，它们的类型会被推断为`any`

```typescript
let a = undefined;   // any
const b = undefined; // any

let c = null;        // any
const d = null;      // any
```

4. 声明为 `null` 类型，但未赋值，默认值为 `undefined`` 而不是 `null`
```typescript
let x:null;
console.log(x)  // 输出 undefined 而不是 null
```

## Object 与 object
### Object
广义对象，少用。除了`undefined`和`null`这两个值不能转为对象，其他任何值都可以赋值给`Object`类型。

```typescript
let obj:Object;
 
obj = true;
obj = 'hi';
obj = 1;
obj = { foo: 123 };
obj = [1, 2];
obj = (a:number) => a + 1;
obj = undefined; // 报错
obj = null; // 报错
```

另外，**空对象`{}`是`Object`类型的简写形式，所以使用`Object`时常常用空对象代替**

```typescript
let obj:{}; // 等价于 let obj:Object;
```

上面示例中，变量`obj`的类型是空对象`{}`，就代表`Object`类型。

显然，无所不包的`Object`类型既不符合直觉，也不方便使用。

### object 类型

狭义对象，常用。只包含对象、数组和函数，不包括原始类型的值。**大多数时候，我们使用的是 `object` 而不是 `Object`**

```typescript
let obj:object;
 
obj = { foo: 123 };
obj = [1, 2];
obj = (a:number) => a + 1;
obj = true; // 报错
obj = 'hi'; // 报错
obj = 1; // 报错
```

## 值类型与联合类型

```ts
// x 的类型是 "https"
const x = 'https';
// y 的类型是 string
const y:string = 'https';
```

变量`x`是`const`命令声明的，TypeScript 就会推断它的类型是值`https`，而不是`string`类型。


**值类型一般不会单独使用，而是结合联合类型，表示值的枚举**

```typescript
let setting:true|false;

let gender:'male'|'female';

let rainbowColor:'赤'|'橙'|'黄'|'绿'|'青'|'蓝'|'紫';
```

上面的示例都是由值类型组成的联合类型，非常清晰地表达了变量的取值范围。其中，`true|false`其实就是布尔类型`boolean`


联合类型除了和值类型一起使用，也可用于多种类型

```typescript
let name:string|number;

name = 'John';
name = 3;
```

“类型缩小”是 TypeScript 处理联合类型的标准方法，凡是遇到可能为多种类型的场合，都需要先缩小类型，再进行处理。实际上，联合类型本身可以看成是一种“类型放大”（type widening），处理时就需要“类型缩小”（type narrowing）

```typescript
function printId(
  id:number|string
) {
  if (typeof id === 'string') {
    console.log(id.toUpperCase());
  } else {
    console.log(id);
  }
}
```

或

```typescript
function getPort(
  scheme: 'http'|'https'
) {
  switch (scheme) {
    case 'http':
      return 80;
    case 'https':
      return 443;
  }
}
```

## type 命令

`type`命令用来定义一个类型的别名

```typescript
type Age = number;

let age:Age = 55;
```

type 不允许重名

```typescript
type Color = 'red';
type Color = 'blue'; // 报错
```

## 数组
**JavaScript 数组在 TypeScript 里面分成两种类型，分别是数组（array）和元组（tuple）**

### 数组的 TS 声明
TypeScript 数组有一个根本特征：所有成员的类型必须相同

```typescript
let arr:number[] = [1, 2, 3];       // 普通写法，推荐
// 或
let arr:Array<number> = [1, 2, 3];  // 泛型写法
```

### 数组的 TS 类型推断

如果变量的初始值是空数组，那么  TypeScript 会推断数组类型是`any[]`。

```typescript
// 推断为 any[]
const arr = [];
```

后面，为这个数组赋值时，TypeScript 会自动更新类型推断。

```typescript
const arr = [];
arr // 推断为 any[]

arr.push(123);
arr // 推断类型为 number[]

arr.push('abc');
arr // 推断类型为 (string|number)[]
```

如果初始值不是空数组，类型推断就不会更新。

```typescript
// 推断类型为 number[]
const arr = [123];

arr.push('abc'); // 报错
```

### 多维数组
TypeScript 使用`T[][]`的形式，表示二维数组，`T`是最底层数组成员的类型。

```typescript
var multi:number[][] =
  [[1,2,3], [23,24,25]];
```

## 元组
元组和数组最大的区别就是，元组的各个成员的类型可以不同。因此它必须明确声明每个成员的类型

```typescript
const s:[string, string, boolean] = ['a', 'b', true];
```

元组成员的类型可以添加问号后缀（`?`），表示该成员是可选的，但必须处于必选成员之后

```typescript
type myTuple = [
  number,
  number,
  number?,
  string?
];

const s:myTuple = [1,2,3]   // 正确
```

由于类型声明是必须的，所以大多数情况元组的元素个数是确定的，但也有例外

```typescript
type t1 = [string, number, ...boolean[]];
type t2 = [string, ...boolean[], number];
type t3 = [...boolean[], string, number];
```

