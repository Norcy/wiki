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
5. 复合属性

## CSS 渲染流程
VideoNative 一套完整的脚本代码（VNML+CSS+JS+JSON），渲染出来的是一个 QVNPage

整个 CSS 的渲染流程如下：

1. Page 打开的时候，将 CSS 文件中的属性读取并解析其值，存放到 QVNContext 的 QVNRichCss 中（注意此时只是属性集，至于会应用到哪些 Node 还不确定，需要根据选择器来匹配；同时为了效率，这个步骤只做一次）
2. 读取 Node 中的内联样式（VNML 中写的），创建一个 ProperyMap
3. Widget 节点
    1. 根据这个 ProperyMap解析 ID 和类名，将 PropertyMap 的值转为其属性集，以此得到内联属性
    2. 处理父亲的属性集，以此得到继承属性
    3. 根据 ID/类名/类型等选择器对 QVNContext 的 QVNRichCss 的样式进行匹配，以此得到从 CSS 文件中来的属性
4. 在将属性应用到 View 节点之前，这里会执行一步整理复合属性
5. 每个 Widget 都对应一个 AttributeSetter，AttributeSetter 会声明其感兴趣的属性。AttributeSetter 拿到属性集之后，会取出其感兴趣的属性，读取其值设置给 View

### 复合属性
首先陈述一个事实，在将 Widget 节点的属性集应用到  View 节点的时候，各个属性是一个接一个应用的（非同时），同时它们应用的顺序是不确定的

如果不同的属性之间有依赖，比如 View 节点拿到 A 属性的时候要根据 B 属性去设置 View，那么此时 B 属性如果还没设置过来，那么就会有问题

此时我们引入一个复合属性的概念，将有依赖的属性封装为一个复合属性，复合属性将做为一个整体设置给 View，以此解决应用顺序不确定的问题

复合属性的子属性的值是来自于原有的独立属性，原有的独立属性的值已经是经过继承、匹配选择器等操作得到的最终值

复合属性对开发者是透明的

### 别名属性
解析过程还涉及一个别名属性的概念，别名属性对开发者也是透明的

以 margin 为例，在 CSS 中，开发者可以写 `margin:1px;` 或 `margin:1px 2px` 或 `margin:1px 2px 3px 4px`，也可以单独写 `margin-top:1px`

而 VideoNative 最终得到的属性集里面，并没有一个叫做 margin 的属性，而是 4 个单独的属性：`marigin-top`, `marigin-left`, `marigin-bottom`, `marigin-right`，margin 属性就是它们的别名属性

#### 别名属性的解析

以 margin-top 属性的解析过程为例，标准的 CSS 属性解析过程是这样的，将传入的 value 通过 parseFunc 解析之后，放到属性集中。其中 parseFunc 负责将 string 转化为 QVNCSSValue，attrPairs 是 Widget 的属性集

```c++
void QVNRichCssAttrType::parse(const std::string &value, IQVNRichCssAttrs &attrPairs) const
{
	QVNCssValue* result = mParseFunc(value);
	attrPairs.putAttribute(*this, result);
}
```

而对 margin 的属性进行解析的时候，先调用别名 CSS 类型 QVNRichCssAliasAttrType 的 parse 方法（QVNRichCssAliasAttrType 是 QVNRichCssAttrType 的子类，所以该方法被重写）

```c++
void QVNRichCssAliasAttrType::parse(const std::string &value, IQVNRichCssAttrs &attrPairs) const
{
	mAliasParseFunc(value, attrPairs, mParseFunc);
}
```

此时调用到了 `VN_RICH_CSS_MARGIN_ALIAS_PARSER`

```c++
static QVNRichCssAliasAttrType::AliasParseFunc VN_RICH_CSS_MARGIN_ALIAS_PARSER = [](const std::string &token, IQVNRichCssAttrs &attrs, QVNRichCssAttrType::ParseFunc parseFunc) -> void
{
	std::vector<std::string> container;
	splitAliasTypes(token, container);
	if (container.size() >= 4) {
		attrs.putAttribute(VN_RICH_CSS_ATTRIBUTE_TYPE_MARGIN_TOP, parseFunc(container[0]));
		attrs.putAttribute(VN_RICH_CSS_ATTRIBUTE_TYPE_MARGIN_RIGHT, parseFunc(container[1]));
		attrs.putAttribute(VN_RICH_CSS_ATTRIBUTE_TYPE_MARGIN_BOTTOM, parseFunc(container[2]));
		attrs.putAttribute(VN_RICH_CSS_ATTRIBUTE_TYPE_MARGIN_LEFT, parseFunc(container[3]));
	}
};
```

别名解析函数将值通过 parseFunc 解析之后，设置给 `margin-top` 等属性
