RN 中，fetch 的 body 参数，只要带上 query，在安卓的 release 版本上，调用 .json() 的时候就会报错。

Android 的 debug 版本没有任何问题；与 `__DEV__` 变量无光；排除了后台代码、回包的影响；只要把 query 改为其他单词比如 check 就没任何问题（后台同步修改）

由于是 Release，默认报错无法查看，复现机型是红米 8A，怀疑是小米系统底层问题。不打算进一步研究

```js
try {
  const rawResponse = await fetch(SERVER_URL, {
    method: 'POST',
    headers: {
        Accept: 'application/json',
        'Content-Type': 'application/json',
    },
    body: JSON.stringify({
        cmd: 'query',   // 罪魁祸首
    }),
  });
  // 这句会报错
  const res = await rawResponse.json();
} catch (e) {
  console.error(e)
}
```