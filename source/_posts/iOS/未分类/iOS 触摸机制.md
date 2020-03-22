## 原理
iOS 把用户触摸事件打包成一个 **UIEvent** 对象，作为事件传递的消息载体，放入当前活跃的 APP 的消息队列中，然后通过 **Hit-Test 机制** 来找到响应者，响应者通过**响应链（Responder Chain）**的传递做出响应，这就是 iOS 事件分发机制的实现原理


## UIEvent 有哪些
UIEvent 包含最常见的三种事件：Touch Events(触摸事件)、Motion Events(运动事件，比如重力感应和摇一摇等)、Remote Events(远程事件，比如用耳机控制手机)。这里我们只讨论触摸事件

## Hit-Test 机制
如图，我点击了 E，Hit-Test 机制是如何找到这个 View 呢？

![Hit-Test](https://user-gold-cdn.xitu.io/2018/1/12/160e94dac2ffc35c)

```objc
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    // 1. 判断能否接受事件
    if (!self.userInteractionEnabled || self.hidden || self.alpha <= 0.01) return nil;

    // 2. 判断是否在当前 View 内
    if ([self pointInside:point withEvent:event] == NO) return nil;

    // 3. 逆序遍历子视图
    for (UIView *subview in [self.subviews reverseObjectEnumerator])
    {
        // 坐标转换，将当前坐标系的点转化为子视图的坐标系的点
        CGPoint childP = [self convertPoint:point toView:subview];
        UIView *hitView = [subview hitTest:childP withEvent:event];
        if (hitView)
        {
            return hitView;
        }
    }

    // 4. 不在子视图，则返回自己
    return self;
}
```

其中 UIView 的 `pointInside:withEvent:` 方法的作用是，判断当前的点是否在当前 View 的 bounds 中

```objc
- (BOOL)pointInside:(CGPoint)point withEvent:(nullable UIEvent *)event
{
    return CGRectContainsPoint(self.bounds, point);
}
```

注意，以下情况，Hit-Test 函数返回 nil

1. view.isHidden = YES
2. view.alpha <= 0.01
3. view.userInterfaceEnable=NO
4. control.enable = NO（如果是 UIControl）

其次注意，子视图的遍历是逆序的，为了保证相同层级下的子视图，离用户越近的优先得到响应

## Responder Chain（响应链）
在 UIKit 中，UIApplication、UIView、UIViewController 这几个类都是直接继承自 UIResponder 类；而响应链是由 UIResponder 组合而成的数组，起始于 FirstResponder，结束于 UIApplication

用户触摸屏幕后，系统通过 Hit-Test 机制找到响应的 UIView，即 FirstResponder；如果该 UIResponder 不处理该事件，则会交给它 的下一个 UIResponder，如果该 UIResponder 处理则停止，否则继续递归直到响应链结束

![](https://user-gold-cdn.xitu.io/2018/1/12/160e94dac3b40720)

1. UIView 的 nextResponder 属性，如果有管理此 view 的 UIViewController 对象，则为此 UIViewController 对象；否则 nextResponder 即为其 superview
2. UIViewController 的 nextResponder 属性为其管理 view 的 superview
3. UIWindow 的 nextResponder 属性为 UIApplication 对象
4. UIApplication 的 nextResponder 属性为 nil。


## 应用
### 寻找 UIView 所在的 Controller
```objc
@implementation UIView (Controller)
- (UIViewController *)viewController
{
    UIResponder *responder = [self nextResponder];
    while (responder)
    {
        if ([responder isKindOfClass:[UIViewController class]])
        {
            return (UIViewController *)responder;
        }
        responder = [responder nextResponder];
    }
    return nil;
}
@end
```

### 扩大按钮点击区域
重写以下方法即可

```objc
- (BOOL)pointInside:(CGPoint)point withEvent:(UIEvent *)event
{
    CGRect relativeFrame = self.bounds;
    // 上下左右扩大 15 像素
    UIEdgeInsets hitTestEdgeInsets = UIEdgeInsetsMake(-15, -15, -15, -15);
    CGRect hitFrame = UIEdgeInsetsInsetRect(relativeFrame, hitTestEdgeInsets);
    return CGRectContainsPoint(hitFrame, point);
}
```

### 子 view 超出了父 view 的 bounds 响应事件
正常情况下，子 View 超出父 View 的 bounds 的那一部分是不会响应事件的

![](https://upload-images.jianshu.io/upload_images/144142-3b8eacb1afb47c93.png)

解决方法1：重写父 View 的 pointInside 方法

这种方法会导致如果点击在父 View （而不是其子 View）上时，不会再响应任何事件，父 View 就像变透明了一样

```objc
- (BOOL)pointInside:(CGPoint)point withEvent:(UIEvent *)event
{
    // 这是默认实现
    // return CGRectContainsPoint(self.bounds, point);

    for (UIView *view in self.subviews)
    {
        if (CGRectContainsPoint(view.frame, point))
        {
            return YES;
        }
    }
    return NO;
}
```

解决方法2：重写父 View 的 hitTest 方法（推荐）

```cpp
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    UIView *view = [super hitTest:point withEvent:event];
    if (view == nil)
    {
        // 如果默认返回 nil，说明此次点击确实不在父 View 范围内
        // 此时我们再多加一层判断是否在子 View 内，如果满足则返回子 View
        for (UIView *subview in [self.subviews reverseObjectEnumerator])
        {
            CGPoint childP = [self convertPoint:point toView:subview];
            UIView *hitView = [subview hitTest:childP withEvent:event];
            if (hitView)
            {
                return hitView;
            }
        }
    }

    return view;
}
```


### 实现一个透明的 View，点击子 View 有效，点击自身无效
```cpp
// 播放器中用到的 QNBPlayerIntellectView
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    UIView *hitTestView = [super hitTest:point withEvent:event];
    // 如果点击的对象是自己，则当没事发生
    if (hitTestView == self)
    {
        hitTestView = nil;
    }
    return hitTestView;
}
```