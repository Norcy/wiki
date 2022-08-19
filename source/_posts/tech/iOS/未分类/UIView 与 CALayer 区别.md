## 联系
1. 每一个 UIView 都有一个根 layer
2. 两者都有**树状层级结构**，CALayer 有 subLayers, UIView 有 subViews

## 区别
1. **触摸**：最大的区别

    CALayer 不能处理用户的触摸事件，而 UIView 可以
    UIView 继承自 UIResponder，可以处理用户的触摸事件
    CALayer 直接继承 NSObject，并没有相应的处理触摸事件的接口

2. **坐标系统**：CALayer 的属性更丰富

    CALayer 的坐标系统比 UIView 多了一个 anchorPoint 属性
    CALayer 具有以下属性：anchorPoint, position, bounds 和 transform 等
    UIView 具有以下属性：bounds, frame 等
    UIView 是由 CoreAnimation 来实现的。它真正的绘图部分，是由一个 CALayer 类来管理
    UIView 本身更像是一个 CALayer 的管理器，访问它的 frame，bounds 等，实际上内部都是在访问它所包含的 CALayer 的相关属性

3. **责任**：UIView 侧重于对显示内容的管理，CALayer 侧重于对内容的绘制

    UIView 和 CALayer 是相互依赖的关系，UIView 依赖 CALayer 绘制的内容，CALayer 依赖 UIView 提供的容器来显示绘制的内容
    所以 CALayer 是基础，没有 CALayer，UIView 自身不会存在，然而，UIView 是 CALayer 的特殊实现，可响应触摸事件

4. **动画**：我们对 UIView 的属性修改时不会产生默认动画，而对单独 layer 属性直接修改会


## 更多详细
+ [UIView 与 CALayer](https://www.cnblogs.com/chenyg32/p/5185619.html)