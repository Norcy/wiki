```tsx
const oldRender = Text.render;
Text.render = (...args) => {
  const origin = oldRender.call(this, ...args);
  return React.cloneElement(origin, {
    style: [fontFamily: '', origin.props.style]
  });
}
```

原理就是重写 Text 组件的 render 方法

React.cloneElement 是 React 一个方法，用于复制一个 React 元素，并为其修改属性或修改 Children

```tsx
function cloneElement<P>(
        element: FunctionComponentElement<P>,
        props?: Partial<P> & Attributes,
        ...children: ReactNode[]
    ): FunctionComponentElement<P>;
```

第一个参数是被拷贝的原元素；第二个参数是希望新增的属性，React 内部会对其和原有属性进行一个合并，而不是覆盖；第三个参数是可选的，是 Children 元素