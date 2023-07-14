## React.memo
React 中，当父组件的一个状态改变后，无论和子组件是否有关，子组件都会受到影响进行重新渲染，这也是 React 中默认的一个行为

使用 React.memo 可以避免子组件多余的渲染，对标类组件的 shouldComponentUpdate

优化的核心思想是，React.memo 会记住组件的输入（props），并在下一次渲染时对比新旧 props 是否发生了变化。如果 props 没有发生变化，React.memo 会阻止组件的重新渲染，直接使用之前的渲染结果


1. 更新父组件的状态时，即使该状态与子组件无关，子组件也会刷新，使用 memo 可以避免多余的刷新

```js
const Child = ({ prop, memo }) => {
  if (memo) {
    console.log("Child 缓存组件渲染")
  } else {
    console.log("Child 组件渲染")
  }
  return <Text>{prop.age}</Text>
}

const MemoChild = React.memo(Child)

export default function App() {
  const [parentData, setParentData] = React.useState(0)
  const [childData, setChildData] = React.useState({
    age: 24,
  })

  return (
    <View style={styles.container}>
      <TouchableOpacity onPress={() => setParentData(parentData + 1)}>
        <Text>{"Update Parent Data"}</Text>
      </TouchableOpacity>
      <TouchableOpacity
        onPress={() => {
          setChildData({ age: childData.age + 1 })
        }}>
        <Text>{"Update Child Data"}</Text>
      </TouchableOpacity>
      <Child prop={childData} />
      <MemoChild
        memo
        prop={childData}
      />
    </View>
  )
}
```


详见例子：https://snack.expo.dev/HVaN5rZ-Z

需要注意的是：

1. React.memo 只能渲染组件，对普通类型无效，普通类型需要用 useMemo 来实现，如

```js
const user = useMemo(() => ({ name: "哈哈" }), []);
```

2. React.memo 的对比使用了浅对比，什么是浅对比？

所谓浅对比，即只对比了第一层 Key，如下例子中，如果只修改了 

```js
const ChildComponent = React.memo(({ person }) => {
  console.log('Rendering ChildComponent');
  return (
    <View>
      <Text>Name: {person.name}</Text>
      <Text>Age: {person.age}</Text>
    </View>
  );
});

const ParentComponent = () => {
  const [data, setData] = React.useState({
    person: { name: 'John', age: 25 },
  });

  const updateData = () => {
    // 创建一个新的数据副本
    const newData = { ...data };
    // 修改年龄
    newData.person.age += 1;
    // 更新状态
    setData(newData);
  };

  return (
    <View>
      <ChildComponent person={data.person} />
      <Button title="Update Data" onPress={updateData} />
    </View>
  );
};
```


如果要自定义对比，可以传入一个比较函数

```js
const isEqual = (prevProps, nextProps) => {
    const prevPropsWithoutMarkDates = omit(prevProps, 'marking');
    const nextPropsWithoutMarkDates = omit(nextProps, 'marking');
    const didPropsChange = some(prevPropsWithoutMarkDates, function (value, key) {
        return value !== nextPropsWithoutMarkDates[key];
    });
    const isMarkingEqual = isEqual(prevProps.marking, nextProps.marking);
    return !didPropsChange && isMarkingEqual;
}
React.memo(Box, isEqual)
```

3. 避免负优化
+ props 每次都会改变的组件，不要使用 React.memo，使用 React.memo 只会带来不必要的新旧 props 比较和无意义的缓存
+ 组件如果很简单，不建议使用 React.memo，并不能带来多大提升，而使用 React.memo 本身就有心智负担

## useMemo
```sh
const memoizedValue = useMemo(() => computeExpensiveValue(a, b), [a, b]);
```

useMemo 接收两个参数，后者是一个数组，类比 useState，当数组内的元素发生变化时才会调用前者传入的函数，一般用于性能优化，减少不必要的函数调用

优化前如下代码，每次更改 children 时都会导致 changeName 的调用

```js
function Button({ name, children }) {
    function changeName(name) {
        return name + "123"
    }

    const otherName = changeName(name)
    return (
        <>
            <div>{otherName}</div>
            <div>{children}</div>
        </>
    )
}
```

优化后，每次更改 children 时，name 由于没有发生没有，changeName 就不会再调用

```js
function Button({ name, children }) {
    function changeName(name) {
        return name + "123"
    }

    // 由于使用了 useMemo，name 没有改变时，changeName 也不会被调用
    const otherName = useMemo(() => changeName(name), [name])
    return (
        <>
            <div>{otherName}</div>
            <div>{children}</div>
        </>
    )
}
```

详见 https://juejin.cn/post/6844903809269891085


