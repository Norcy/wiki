## 剩余参数
使用 & 来声明 props 的类型

```ts
const ListItem = ({
  item,
  ...props
}: {
  item: UIRecord;
} & ViewProps) => {
  return <View ...props/>
}
```

## children
```ts
children?: React.ReactNode | undefined;
```

## 字典的 key
```ts
const foo: {[key: string]: string} = {
  'a': '123'
}
```

## useRef
```tsx
const allPageData = useRef<UIRecord[]>([]);
allPageData.current // 此时为 UIRecord[]
```

```tsx
const listRef = useRef<FlatList|null>();
<FlatList
  ref={(ref) => (listRef.current = ref)}
/>
listRef.current?.scrollToOffset(0)
```

## type
普通 type

```ts
export type FooType = {
  marked: boolean;
  image?: string;
};
```

type 联合

```ts
export type FooType = {
  marked: boolean;
  image?: string;
} & ViewProps;
```

type 联合 2

```ts
interface OriginProps = {
  name: string;
}

interface CustomProps extends OriginProps = {
  age: number;
}

// Before
// export type FooType = {
//   [key:string]: OriginProps;
// };

// After
export type FooType = {
  [key:string]: CustomProps;
};
```

## 类型强转
```ts
const date: DateData = data.date as DateData;
```