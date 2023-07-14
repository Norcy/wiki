## useRef
useRef 有两个作用，详细可参考 https://zh-hans.reactjs.org/docs/hooks-reference.html#useref

### 1. 为子组件提供命令式访问
通常配合子组件的 ref 属性使用

```js
function TextInputWithFocusButton() {
  const inputEl = useRef(null);
  const onButtonClick = () => {
    // `current` 指向已挂载到 DOM 上的文本输入元素
    inputEl.current.focus();
  };
  return (
    <>
      <input ref={inputEl} type="text" />
      <button onClick={onButtonClick}>Focus the input</button>
    </>
  );
}
```

### 2. 提供实例变量，减少渲染次数
useRef() 和自建一个 {current: ...} 对象的唯一区别是，useRef 会在每次渲染时返回同一个 ref 对象。注意自建对象在每次渲染的时候都保持其初始值，而 useRef 会保持上一次渲染时候的值

请记住，当 ref 对象内容发生变化时，useRef 并不会通知你。变更 .current 属性不会引发组件重新渲染。如果想要触发渲染，则需要手动调用


老方案：每次选择标签会触发两次 Render

```js
const [rateFilterIndex, setRateFilterIndex] = useState(0)

useEffect(()=> {
    // reloadData 会触发第二次渲染
    reloadData();
}, [rateFilterIndex])

<MyTagFilter
    selectIndex={rateFilterIndex}
    didFilterSelect={(newIndex) => {
        // setRateFilterIndex 会触发第一次渲染
        setRateFilterIndex(newIndex)
    }}
/>
```

新方案：使用 useRef 减少 Render 次数，每次选择标签只会触发一次 Render

```js
const rateFilterIndex = useRef(0)

<MyTagFilter
    selectIndex={rateFilterIndex.current}
    didFilterSelect={(newIndex) => {
        // reloadData 会重新触发渲染
        rateFilterIndex.current = newIndex;
        reloadData()
    }}
/>
```

## react navigation 的 focus 回调中的 state 没有及时更新

以下代码，假如此刻 setRoutes 已经触发，有新的值了，然后再切换 Tab 触发 focus 回调

此时下面这代码的 focus 回调里面会一直使用老的 routes

```js
const [routes, setRoutes] = useState({a:1})

React.useEffect(() => {
    const unsubscribe_focus = navigation.addListener('focus', () => {
        // 打印出来的是老的 routes
        console.log('Home viewWillAppear', routes);
    });
    return function cleanup() {
        unsubscribe_focus();
    };
}, [navigation]);
```

解决办法是在 useEffect 的参数里面添加 routes 监听，此时下面这代码的 focus 回调里面会读到新的 routes

```js
React.useEffect(() => {
    const unsubscribe_focus = navigation.addListener('focus', () => {
        // 打印出来的是新的 routes
        console.log('Home viewWillAppear', routes);
    });
    return function cleanup() {
        unsubscribe_focus();
    };
}, [navigation, routes]);
```


## 获取 value 总是老的值
```js
// 添加 useLayoutEffect 会导致点击函数中获取的 value 值永远是老的
// React.useLayoutEffect(() => {
    navigation.setOptions({
        headerTitle: '编辑' + title,
        headerRight: () => _renderSaveButton(),
    });
// }, [navigation]);
```

## setState 失效
1. 如果值是一个数组，记得拷贝一下 slice()，否则即使对象的值改了，state 也不会变化

```js
console.log(pageData.data) // print 111
pageData.data = 222
console.log(pageData.data) // print 222
setPageData(pageData) // 不会触发 UI 刷新！
setPageData(pageData.slice()) // 这样才会触发 UI 刷新！
```

2. 如果值是一个对象，则不能使用 `slice`，而使用 `...`

```js
setPageData({...pageData})
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

可以看到，setState 这个方法，并不是同步生效的！

那么使用 await 来等待 setState 是否可行呢？

```js
const onPageDataUpdate = async (newPageData) => {
	await setPageData(newPageData)
	const searchResult = searchInPageData(pageData)	// 此时我们期望 pageData 是对的，但其实是老数据！
	setSearchResult(searchResult)	// 利用 searchResult 更新 UI，UI 使用的是 {searchResult}
}
```

结果依然是不对的，这就说明，**await 操作并不能让 setState 等待**，详细可以看这个 [demo](https://codesandbox.io/s/ypy7ny58kz)

正规做法是需要借助 useEffect 来完成这个操作（如果是 setState，则使用 setState 的第二个参数来回调）

使用 useEffect 的代码如下

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

## useEffect 里面如何调用 async 函数
```js
useEffect(() => {
    (async () => {
      await initConnection();
      console.log('iap initConnection finish');
    })();

    return () => {
      endConnection();
    };
  }, []);
```

## React Context 学习
当主题设置、语言设置等类似需求时，使用 Context 是一个非常好的解决方案。Context 主要解决属性层层传递的问题，当数据源变更时，可以方便的通知到各个使用该属性的地方

主要包含三个方法：

1. createContext

    ```js
    const MyContext = React.createContext(defaultValue);
    ```

    主要用于创建 Context，支持传递 defalutValue

2. Provider

    ```js
    <MyContext.Provider value={customValue}>
    ```

    被 Provider 包裹的组件，无论递归多少，都能方便的使用 Context 的 value

    Provider 标签支持传递 value，如果不传递，则使用 createContext 中的 defaultValue

3. Consumer

    ```js
    <MyContext.Consumer>
    {(value) => {
        console.log("value 变化了");
    }}
    </MyContext.Consumer>   
    ```

    Consumer 的作用是订阅 context 的变化


### 注意事项 1：并不是所有的数据层层传递是适合用 Context

比如，考虑这样一个 Page 组件，它层层向下传递 user 和 avatarSize 属性，从而让深度嵌套的 Link 和 Avatar 组件可以读取到这些属性：

```js
<Page user={user} avatarSize={avatarSize} />
// ... 渲染出 ...
<PageLayout user={user} avatarSize={avatarSize} />
// ... 渲染出 ...
<NavigationBar user={user} avatarSize={avatarSize} />
// ... 渲染出 ...
<Link href={user.permalink}>
  <Avatar user={user} size={avatarSize} />
</Link>
```

如果在最后只有 Avatar 组件真的需要 user 和 avatarSize，那么层层传递这两个 props 就显得非常冗余。而且一旦 Avatar 组件需要更多从来自顶层组件的 props，你还得在中间层级一个一个加上去，这将会变得非常麻烦。

### 注意事项 2：避免多余渲染
Provider 的 value 是浅比较，所以当 Provider 重新渲染的时候，由于 value 属性总是被赋值为新的对象，以下的代码会重新渲染下面所有的 consumers 组件

```js
class App extends React.Component {
  render() {
    return (
      <MyContext.Provider value={{something: 'something'}}>
        <Toolbar />
      </MyContext.Provider>
    );
  }
}
```

为了防止这种情况，将 value 状态提升到父节点的 state 里：

```js
class App extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      value: {something: 'something'},
    };
  }

  render() {
    return (
      <MyContext.Provider value={this.state.value}>
        <Toolbar />
      </MyContext.Provider>
    );
  }
}
```

### 注意事项 3：Hook 下的 context
1. 依然使用 Provider 包裹
2. 使用 useContext 能够读取 context 的 value，以及代替 Consumer 监听 context 变化
3. useContext 的入参和返回值都是 context 本身
4. 如何从子组件更新 Context 的内容，主要是将 setState 传到 context 中，详见 https://stackoverflow.com/questions/41030361/how-to-update-react-context-from-inside-a-child-component
