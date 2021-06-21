## useRef 
https://zh-hans.reactjs.org/docs/hooks-reference.html#useref
https://zhuanlan.zhihu.com/p/105276393

## setState 的时候，如果值是一个对象，记得拷贝一下 slice()，否则即使对象的值改了，state 也不会变化

```js
console.log(pageData.data) // print 111
pageData.data = 222
console.log(pageData.data) // print 222
setPageData(pageData) // 不会触发 UI 刷新！
setPageData(pageData.slice()) // 这样才会触发 UI 刷新！
```

## 如何正确使用 useState 来更新 UI

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

## setState 的四种用法
1. 接受一个 Object

setState 方法其实是 “异步” 的。即立马执行之后，是无法直接获取到最新的 state 的

```js
console.log(this.state.count) // 0
this.setState({count: this.state.count + 1})
console.log(this.state.count) // 0
```

2. 接受一个 Object 和函数

setState 的回调可以获取最新的 state 的值

```js
this.setState({count: this.state.count + 1}, ()=>{
    console.log(this.state.count) // 1
})
```

3. 只接受一个函数

函数的首个参数就是上一次的 state，利用这个可以实现连续“同步”更新 state

```js
this.setState(prevState => {count: prevState.count + 1});	// 0
this.setState(prevState => {count: prevState.count + 1});	// 1
```

4. 接受两个函数

第一个函数的首个参数是上一次的 state，第二个函数是 callback

```js
this.setState(prevState => {count: prevState.count + 1}, () => {
	console.log("更新完成")
});
```

## setState(object) 是真的异步吗？

1. 在生命周期（Hook 函数）中是异步的

```js
class MyTest extends Component {
    state = {
        count: 0
    };

    componentDidMount() {
        console.log(this.state.count)	// 0
        this.setState({ count: this.state.count + 1 })
        console.log(this.state.count)	// 0
    }
}
```

2. 在合成事件中是异步的

```js
class MyTest extends Component {
    state = {
        count: 0
    };

	render() {
        return (
            <TouchableWithoutFeedback onPress={() => {
                console.log(this.state.count)	// 0
                this.setState({ count: this.state.count + 1 })
                console.log(this.state.count)	// 0
            }} >
                <View />
            </TouchableWithoutFeedback >
        )
    }
}
```

3. 在 setTimeout 中是同步的

```js
class MyTest extends Component {
    state = {
        count: 0
    };

	componentDidMount() {
        setTimeout(() => {
            console.log(this.state.count)	// 0
            this.setState({ count: this.state.count + 1 })
            console.log(this.state.count)	// 1
        }, 1);
    }
}
```

除了 setTimeout 外，在 DeviceEventEmitter 的回调函数里，也是同步执行。这跟 js 的事件循环机制有关，setTimeout 和 DeviceEventEmitter 的回调都是在事件循环结束时调用，其余时候调用 setState 则是异步的。

https://github.com/sisterAn/blog/issues/26

## SVG 的替换颜色不生效

```sh
yarn start --reset-cache
```


## useEffect 的作用

```js
// 没有参数，每一次 Render 之后都会执行
useEffect(() => {
    console.log('每一次 Render 之后都会执行')
})

// 空数组，只会执行一次
useEffect(() => {
    console.log('只会执行一次')
}, [])

// 数组单个值，特定参数改变的时候都会执行（该值是 state）
useEffect(() => {
    console.log('navigation 改变的时候都会执行')
}, [navigation])

// 数组多个值，每个值改变的时候都会执行（该值是 state）
useEffect(() => {
    console.log('id 或 name 改变的时候都会执行')
}, [id, name])
```