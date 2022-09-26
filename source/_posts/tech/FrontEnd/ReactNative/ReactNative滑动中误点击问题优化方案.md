转自公司内文章

## 背景
使用 TouchableOpacity 组件，在快速横向切换和上下滑动过程中，存在较高概率的误点击的情况

RN 的点击原理如下

![原理图](https://static.yximgs.com/udata/pkg/EE-KSTACK/e31bd613130b81d3fab3054e66762417.svg)

## 方案
根据以上原理，我们可以在 onPressIn 的时候记录下手势坐标，然后在 onPress 时进行判断，如果有滑动（手势在 x或者 y 方向上移动了一段距离），则不触发 onPress 事件，从而解决滑动过程中误点击的问题。

```js
import React, {useRef} from "react";
import {
    TouchableOpacity,
} from "react-native";

const PREVENT_DISTANCE = 2;

export default function FixedTouchableOpacity({
    onPress,
    onPressIn,
    ...props
}) {
    const _touchActivatePositionRef = useRef(null);

    const _onPressIn = (e: any) => {
        const { pageX, pageY } = e.nativeEvent;

        _touchActivatePositionRef.current = {
            pageX,
            pageY,
        };

        onPressIn?.(e);
    }

    const _onPress = (e: any) => {
        const { pageX, pageY } = e.nativeEvent;

        const absX = Math.abs(_touchActivatePositionRef.current.pageX - pageX);
        const absY = Math.abs(_touchActivatePositionRef.current.pageY - pageY);

        const isDragging = absX >= PREVENT_DISTANCE || absY >= PREVENT_DISTANCE;
        if (!isDragging) {
            onPress?.(e);
        }
    }

    return (
        <TouchableOpacity onPressIn={_onPressIn} onPress={_onPress} {...props}>
            {props.children}
        </TouchableOpacity>
    );
}
```