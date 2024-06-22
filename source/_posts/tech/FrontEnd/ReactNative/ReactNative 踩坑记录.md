## di18n 失效
字符串中只要出现 "." 就会失效，比如

```tsx
// 翻译失败
i18n.t("该文件的后缀名不对，应该为 .db")
// 翻译成功
i18n.t("该文件的后缀名不对，应该为 db")
```

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


## Android TextInput 键盘弹出出现空白问题
1. AndroidMainfest.xml 修改 windowSoftInputMode 从 adjustPan 改为 adjustResize，即 `android:windowSoftInputMode="adjustResize"`
2. KeyboardAvoidingView 的 behavior 选择 height 而不是 padding

```tsx
<KeyboardAvoidingView
behavior={Platform.OS == 'ios' ? 'padding' : 'height'}>
...
```

## Android TextInput 第二次键盘弹起时，cursor 正确，但是没有自动自动滚动到 cursor，必须先输入一下之后才会滚动

解决方法：模拟用户操作 cursor

```tsx
const cursorPosition = React.useRef(0);

useEffect(() => {
  const unsubscribe = Keyboard.addListener('keyboardDidShow', (e) => {
    if (Platform.OS === 'android') {
      setTimeout(() => {
        inputRef.current?.setNativeProps({
          selection: {
            start: cursorPosition.current,
            end: cursorPosition.current,
          },
        });
        // 必须设置回来，不然 cursor 永远不对
        inputRef.current?.setNativeProps({
          selection: {},
        });
      }, 10);
    }
  });
  
  return () => {
    unsubscribe.remove();
  };
}, []);

<TextInput
  onSelectionChange={({
    nativeEvent: {
      selection: {start, end},
    },
  }) => {
    cursorPosition.current = start;
  }}
>
```



## Android TextInput cursor 不对的问题
TextInput 在 multiline，首次 focus 时，cursor 不是在手指的位置，而是在文本末尾

解决方法：

```tsx
const inputRef = React.useRef<TextInput | null>();

useEffect(() => {
  // 完美解决安卓首次 focus 聚焦到 Text 末尾的问题
  setTimeout(() => {
    if (Platform.OS === 'android') {
      inputRef.current?.focus();
      inputRef.current?.blur();
    }
  }, 10);
}, []);

<TextInput
  multiline={true}
  ref={(ref) => (inputRef.current = ref)}
/>
```

## iOS TextInput 在换行时会滚动到顶部
复现路径1：文本超过一屏，最好全部是中文，最后一行倒数几个是英文，输入一个个英文，会发现换行时，TextInput 自动滚动到顶部，再输入一下才会滚回来

复现路径2：文本超过一屏，最好全部是中文，最后一行倒数几个留空，一次性输入大段文字（可通过拼音转汉字或者粘贴实现）确保换行，此时也会复现，也是再输入一下才会滚回来

思路：因为再输入一下才会滚回来，所以模拟用户输入可以解决该问题。每次当新换行时，输入一个零宽字符，再延时移除，即可解决该问题

```tsx
const oldHeight = React.useRef(-1);
const ZeroWidthCharRegex = /[\u200B]/;
const [comment, setComment] = useState('');

<TextInput
  onTextChange={(name, value) => {
    if (Platform.OS == 'ios' && ZeroWidthCharRegex.test(value)) {
      // 如果打字过快，可能会走到这，需要移除零宽字符
      value = value.replace(ZeroWidthCharRegex, '');
    }
    setComment(value);
  }}
  onContentSizeChange={({
    nativeEvent: {
      contentSize: {width, height},
    },
  }) => {
    if (Platform.OS == 'ios' && height > oldHeight.current) {
      const currentValue = comment;
      if (!ZeroWidthCharRegex.test(currentValue)) {
        // 新增零宽字符
        // console.log('新增零宽字符');
        // 第一个 timeout 是为了解决大段拼音转文字后，出现回滚顶部的情况
        setTimeout(() => {
          setComment(comment + '\u200B');
          // 第二个 timeout 是为了增加零宽字符后，不要立刻移除，否则不会触发 TextInput 的自动滚动
          setTimeout(() => {
            // 移除零宽字符
            // console.log('移除零宽字符');
            setComment(comment.replace(ZeroWidthCharRegex, ''));
          }, 10);
        }, 10);
      }
    }
    oldHeight.current = height;
  }}
/>
```