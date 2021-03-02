1. **setState 的时候，如果值是一个对象，记得拷贝一下 slice()，否则即使对象的值改了，state 也不会变化**

```js
console.log(pageData.data) // print 111
pageData.data = 222
console.log(pageData.data) // print 222
setPageData(pageData) // 不会触发 UI 刷新！
setPageData(pageData.slice()) // 这样才会触发 UI 刷新！
```

2. 如何正确使用 useState 来更新 UI

我们想在 pageData 更新的时候，更新下搜索结果。

错误的代码

```js
const onPageDataUpdate = async (newPageData) => {
	setPageData(newPageData)
	const searchResult = searchInPageData(pageData)	// 此时我们期望 pageData 是对的，但其实是老数据！
	setSearchResult(searchResult)	// 利用 searchResult 更新 UI，UI 使用的是 {searchResult}
}
```

可以看到，setState 这个方法，并不是同步生效的！我们需要借助 useEffect 来完成这个操作

正确的代码

```js
useEffect(() => {
	const searchResult = searchInPageData(pageData)	// 此时 pageData 是对的
	setSearchResult(searchResult)	// 利用 searchResult 更新 UI，UI 使用的是 {searchResult}
}, [pageData])	// 当 pageData 变化时，调用该函数

const onPageDataUpdate = async (newPageData) => {
	setPageData(newPageData)	// 只更新 pageData，然后等待 useEffect 的更新
}
```


3. SVG 的替换颜色不生效

```sh
yarn start --reset-cache
```