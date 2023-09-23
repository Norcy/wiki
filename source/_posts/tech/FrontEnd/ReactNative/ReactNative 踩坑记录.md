## FlatList 的 Header 中的 TextInput，每次输入后总是会失去焦点
如果 Header 中含有 TextInput，则不能返回一个渲染函数，否则 TextInput 会每次输入完，重新渲染，从而失去焦点。
https://github.com/facebook/react-native/issues/13365

## TextInput 在 Editable 为 false 时会响应长按
pointerEvents 会禁止所有手势

```js
<View pointerEvents={isEdit ? 'none' : 'auto'}>
  <TextInput editable={isEdit}>
</View>
```

## 慎用 length 代替 bool
```jsx
{media.url?.length && _renderUrlButton()}
```

该代码 length 返回的是 int，用 int 来作为 && 的条件，是有问题的，会导致报错 

```sh
Text strings must be rendered within a <Text> component.
```

解决办法

```jsx
{media.url?.length > 0 && _renderUrlButton()}
```
