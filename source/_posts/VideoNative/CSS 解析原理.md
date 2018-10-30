---
title: CSS 解析原理
---

## CSS 属性的分类
VideoNative 的 CSS 属性可以从以下几个来源获得：

1. VNML 的标签（Node）中写的内联样式
    
    ```html
    <text style="color: red">
    This is a text
    </text>
    ```
2. CSS 文件中的样式，即经过选择器匹配后的 CSS 属性 

    ```css
    .myTitle
    {
        height:auto;
        width:auto;
        font-size:26rpx;
        color:#848494;
    }
    ```
3. 继承属性
4. 默认属性

## CSS 渲染流程
VideoNative 一套完整的脚本代码（VNML+CSS+JS+JSON），渲染出来的是一个 QVNPage

整个 CSS 的渲染流程如下：

1. Page 打开的时候，将 CSS 文件中的属性读取并解析其值，存放到 QVNContext 的 QVNRichCss 中（注意此时只是属性集，至于会应用到哪些 Node 还不确定，需要根据选择器来匹配；同时为了效率，这个步骤只做一次）
2. 读取 Node 中的内联样式，创建一个 ProperyMap
3. Widget 节点
    1. 根据这个 ProperyMap解析 ID 和类名，将 PropertyMap 的值转为其属性集，以此得到内联属性
    2. 处理父亲的属性集，以此得到继承属性
    3. 根据 ID/类名/类型等选择器对 QVNContext 的 QVNRichCss 的样式进行匹配，以此得到从 CSS 文件中来的属性
    4. 根据得到的属性集进行事件监听的解析
4. 在将属性应用到 View 节点之前，这里会做一步整理复合属性，

### 复合属性
首先陈述一个事实，在将 Widget 节点的属性集应用到  View 节点的时候，各个属性不是同时应用的，它们应用的顺序是不确定的。

如果不同的属性之间有依赖，比如 View 节点拿到 A 属性
举个例子，Video 标签中，我们会将属性集中的 src/autoplay 等属性整合成一个复合属性 video 添加到属性集中。这样的好处是将来
复合属性对开发者是透明的，复合属性的好处是，在计算 CSS 的阶段可以将属性聚合，如果散开设置给 View

## JS 更改属性
属性不是一成不变的，开发者可以通过 JS 修改属性

```js
var text = vn.dom.getElementById("text");
text.setPropert("hidden", true);
```

通过 JS 设置的属性的流程如下

