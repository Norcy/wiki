## ScrollView 如何让内容撑满
```js
<ScrollView
contentContainerStyle={{ flexGrow: 1 }}
>
</ScrollView>
```

如何实现上拉加载

```js
<ScrollView
contentContainerStyle={{ flexGrow: 1 }}
>
<!-- 你的数据 -->
...
<!-- 占位 -->
<View style={{flex: 1, width: '100%'}}></View>
<View>
<Text
  style={{
    marginTop: 30,
    position: 'absolute',
    textAlign: 'center',
    width: '100%',
    color: 'gray',
  }}>
  {'继续上拉查看时间轴↑'}
</Text>
</View>
</ScrollView>
```


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

## 在 JSX 中慎用 length 作为 && 的条件
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


如果是普通的 if 条件，或者是三元表达式就没问题

```jsx
{media.url?.length ? _renderUrlButton() : </>} // 没问题
```

```js
// 这样也没问题
if (media.url?.length && someCondition) {

}
```