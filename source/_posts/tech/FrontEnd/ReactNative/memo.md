## React.memo

React 中，当父组件的一个状态改变后，无论和子组件是否有关，子组件都会受到影响进行重新渲染，这也是 React 中默认的一个行为

使用 React.memo 可以避免子组件多余的渲染，对标类组件的 shouldComponentUpdate

优化的核心思想是，React.memo 会记住组件的输入（props），并在下一次渲染时对比新旧 props 是否发生了变化。如果 props 没有发生变化，React.memo 会阻止组件的重新渲染，直接使用之前的渲染结果

详见例子：https://snack.expo.dev/@tt123753/memo

1. 更新父组件的状态时，即使该状态与子组件无关，子组件也会刷新，使用 memo 可以避免多余的刷新

```js
const Child = ({ prop, memo }) => {
  if (memo) {
    console.log("Child 缓存组件渲染");
  } else {
    console.log("Child 组件渲染");
  }
  return <Text>{prop.age}</Text>;
};

const MemoChild = React.memo(Child);

export default function App() {
  const [parentData, setParentData] = React.useState(0);
  const [childData, setChildData] = React.useState({
    age: 24,
  });

  return (
    <View style={styles.container}>
      <TouchableOpacity onPress={() => setParentData(parentData + 1)}>
        {/*只触发 Child*/}
        <Text>{"Update Parent Data"}</Text>
      </TouchableOpacity>
      <TouchableOpacity
        onPress={() => {
          setChildData({ age: childData.age + 1 });
        }}
      >
        {/*触发 Child 和 MemoChild*/}
        <Text>{"Update Child Data"}</Text>
      </TouchableOpacity>
      <Child prop={childData} />
      <MemoChild memo prop={childData} />
    </View>
  );
}
```

2. 但如果子组件是定义在父组件中，那么每次父组件刷新时，子组件总是会重新渲染，即使加了 memo，这一点要特别注意

```jsx
export default function App() {
  const [parentData, setParentData] = React.useState(0);

  const Child = ({ prop, memo }) => {
    if (memo) {
      console.log("Child 缓存组件渲染");
    } else {
      console.log("Child 组件渲染");
    }
    return <Text>{prop.age}</Text>;
  };

  const MemoChild = React.memo(Child);

  return (
    <View style={styles.container}>
      <TouchableOpacity onPress={() => setParentData(parentData + 1)}>
        {/*触发 Child 和 MemoChild*/}
        <Text>{"Update Parent Data"}</Text>
      </TouchableOpacity>
      <Child prop={childData} />
      <MemoChild memo prop={childData} />
    </View>
  );
}
```

3. React.memo 的对比函数

memo 默认使用了浅对比，什么是浅对比？即只要引用的对象变了，就会触发刷新，但其实这种刷新可能是多余的。

如下例，如果不引入 isEqual 函数，

```js
const isEqual = (prevProps, nextProps) => {
  return prevProps.age == nextProps.age;
};

const Child = ({ prop, memo }) => {
  if (memo) {
    console.log("Child 缓存组件渲染");
  } else {
    console.log("Child 组件渲染");
  }
  return <Text>{prop.age}</Text>;
};

const MemoChild = React.memo(Child, isEqual);

export default function App() {
  const [childData, setChildData] = React.useState({
    age: 24,
  });

  return (
    <View style={styles.container}>
      <TouchableOpacity
        onPress={() => {
          {
            /*age 的值其实没有改变，但是 MemoChild 引用的对象变了*/
          }
          setChildData({ age: childData.age });
        }}
      >
        {/*引入 isEqual 后只触发 Child，否则两个都触发*/}
        <Text>{"Update Child Data"}</Text>
      </TouchableOpacity>
      <Child prop={childData} />
      <MemoChild memo prop={childData} />
    </View>
  );
}
```

4. 不适用 React.memo 的情况

- props 每次都会改变的组件，不要使用 React.memo，使用 React.memo 只会带来不必要的新旧 props 比较和无意义的缓存
- 组件如果很简单，不建议使用 React.memo，并不能带来多大提升，而使用 React.memo 本身就有心智负担
- React.memo 只能针对函数组件，对于普通函数的优化得用 useMemo

## 配合 useCallback 完成优化
https://juejin.cn/post/7107943235099557896

改造前：刷新与 Child 无关的 Parent 属性时，Child 会触发渲染，原因是每次刷新时，传入 Child 的函数都会重新构建，而 Memo 比较的是函数的地址，所以会重刷 Child

```js
const calRef = useRef(null);
return (
  <MemoChild ref={(ref) => {chartRef.current = ref}} />;
)
```

改造后：解决方法是使用 useCallback 保证函数地址不变，从而不会触发 Memo 的舒心。改造后刷新与 Child 无关的 Parent 属性时，Child 不再触发渲染

```js
const calRef = useRef(null);
const setChartRef = useCallback((ref) => {
  chartRef.current = ref;
}, []);

return (
  <MemoChild ref={setChartRef} />;
)
```

需要注意的是，使用 useCallback 会导致原函数捕捉的 state 不再改变，因此如果原函数需要读取 Parent 的 state 时，需要特殊处理下，有两种方法

+ 方法 1：将 state 作为原函数参数传入，在调用该函数时，传入最新的 state 即可

```js
const [count, setCount] = useState(0)
const foo = useCallback((count) => console.log(count), [])
```

+ 方法 2：将 state 作为 useCallback 的第二个依赖参数传入，这样当 state 改变时，useCallback 会重新触发原函数的生成

```js
const [count, setCount] = useState(0)
const foo = useCallback(() => console.log(count), [count])
```



## useMemo

某些场景下，我们只是希望 component 的局部不要重新渲染，而不是整个组件不重新渲染，此时就得用到 useMemo；另外 React.memo 针对的是函数式组件，如果要优化普通的函数执行，则得依赖 useMemo

一句话总结 useMemo 的作用就是，**减少组件重新渲染时不必要的函数计算**

注意一些简答的计算比如 for 循环相加 1000 次这种简单的函数就不要使用 useMemo 了，必须是复杂的函数运算才值得用

useMemo 的基础用法如下

```js
const memoizedValue = useMemo(() => return a+b, [a, b]);
```

useMemo 接收两个参数，后者是一个数组，类比 useState，当数组内的元素发生变化时才会调用前者传入的函数

我们假设 calculateName 是一个非常耗时的函数。如下代码，每次更改 age 时都会导致 calculateName 的调用

```js
const Child = ({ prop }) => {
  const { age, name } = prop;
  {
    /*假如这个函数的运算非常复杂*/
  }
  const calculateName = (name) => {
    console.log("calculateName");
    return name + "123";
  };
  const realName = calculateName(name);
  return (
    <>
      <Text>{age}</Text>
      <Text>{realName}</Text>
    </>
  );
};

export default function App() {
  const [childData, setChildData] = React.useState({
    age: 24,
    name: "Mike",
  });

  return (
    <View style={styles.container}>
      <TouchableOpacity
        onPress={() => {
          setChildData({ age: childData.age + 1, name: "Mike" });
        }}
      >
        <Text>{"Update Child Data"}</Text>
      </TouchableOpacity>
      <Child prop={childData} />
    </View>
  );
}
```

由于 calculateName 只与 name 有关，age 改变重新执行 calculateName 是多余的，因此这种情况下可以使用 useMemo 来减少 calculateName 的调用

```js
const Child = ({ prop }) => {
  const { age, name } = prop;
  {
    /*假如这个函数的运算非常复杂*/
  }
  const calculateName = (name) => {
    console.log("calculateName");
    return name + "123";
  };
  const realName = React.useMemo(() => calculateName(name), [name]);
  return (
    <>
      <Text>{age}</Text>
      <Text>{realName}</Text>
    </>
  );
};

export default function App() {
  const [childData, setChildData] = React.useState({
    age: 24,
    name: "Mike",
  });

  return (
    <View style={styles.container}>
      <TouchableOpacity
        onPress={() => {
          setChildData({ age: childData.age + 1, name: "Mike" });
        }}
      >
        <Text>{"Update Child Data"}</Text>
      </TouchableOpacity>
      <Child prop={childData} />
    </View>
  );
}
```
