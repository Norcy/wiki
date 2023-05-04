```js
const FOO = '123';

const BAR = {
  FOO: 333
};
```

BAR 的结果竟然是 `{FOO:333}`

正确给 Object 设置 key 应该是这样

```js
const FOO = '123';

const BAR = {
  [FOO]: 333
};
```

这样 BAR 的输出为 `{123:333}`，才符合预期