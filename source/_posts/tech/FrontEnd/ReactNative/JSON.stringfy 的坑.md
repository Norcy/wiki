Object 经过 JSON.stringify 转换之后，会丢失值为 undefined 的键值对

```js
let a = {foo: 1, bar: undefined}
JSON.stringify(a) // 输出 '{"foo":1}'
```