## Native Debug
1. 在点击函数里面 console.log(a.b)
+ 如果是 JS Debug，则 Fatal
+ 如果是 JS Release，则打印 undefined

JS 经过编译之后，a 应该是一个存在的变量！

1. 在点击函数里面 console.log(aaa.bbb)
直接 Fatal

2. 在全局，即函数外面 console.log(aaa.bbb)
直接 Fatal

3. 在点击函数里面直接写多余的 abcd
直接 Fatal

3. 随意修改 main.jsbundle
Bundle 加载失败

3. console.error(123)
Soft

## Native Release


```js
ErrorUtils.setGlobalHandler((error: any) => {
    console.log('js error2: ', error);
});
```

Release 下的 Fatal 错误，如果不实现 RN 的 Fatal 回调，默认情况下是会 Crash 的

### 如何制造 Soft
1. console.error
2. 运行起来之后修改代码，使其编译不过的都是 Soft（没什么意义好像，因为打包的时候会编译不过）



### 如何制造 krn_page_error

在 JS 的全局区域写几个字母，比如

```js
const store = createStore(
    rootReducer,
    applyMiddleware(thunkMiddleware, promiseMiddleware),
);

asd
xax

const persistor = persistStore(store);
```

### 如何制造 krn_bundle_load_result 的失败
运行时修改 path 为空
