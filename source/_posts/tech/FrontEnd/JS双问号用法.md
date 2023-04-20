
1. 当左值为 null 或 undefined 时（并非所有 false 值，如 0 或 ''），返回右值
2. 但是当左值为 0 时，依然返回左值，这是和 `||` 的唯一区别

```js
let foo = null ?? 'default string'; //"default string"
foo = 0 ?? 42;  //0
foo = 1 ?? 42;  //1
foo = '' ?? 42;  //''
foo = null || 'default string'; ////"default string"
foo = 0 || 42;  // 42
foo = 1 || 42;  // 1
foo = '' || 42;  //42
```

参考：[Nullish coalescing operator (??)](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Nullish_coalescing)