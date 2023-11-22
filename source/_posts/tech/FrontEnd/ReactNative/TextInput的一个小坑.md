这样写，键盘的修改不会导致 value 改变，你还需要监听 onChangeText 来 setValue

```tsx
const [value, setValue] = useState('')

<TextInput
  value={value}>
</TextInput>
```

但是这样写，就不需要监听，不需要 setValue 了

```tsx
const [value, setValue] = useState('')

<TextInput>
  {value}
</TextInput>
```