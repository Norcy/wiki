1. Info.plist 中设置 `View controller-based status bar appearance` 控制 App 状态栏显隐接受全局配置（NO）或者各控制器各自配置（YES）

    + `View controller-based status bar appearance = NO` 时，使用 `[[UIApplication sharedApplication] setStatusBarHidden:hidden]` 来控制全局状态栏的显隐

    + `View controller-based status bar appearance = YES` 时，重写 ViewController 的 prefersStatusBarHidden 方法来决定当前 Controller 的状态栏显隐


2. 设置 Status bar is initially hidden -> YES 可以隐藏启动页展示过程的状态栏；默认不隐藏

3. ViewController 的相关方法

```objc
// 样式
- (UIStatusBarStyle)preferredStatusBarStyle
{
    return [self.visibleViewController preferredStatusBarStyle];
}

// 显隐
- (BOOL)prefersStatusBarHidden 
{
    return [self.visibleViewController prefersStatusBarHidden];
}

// 标记状态栏需要更新，同 setNeedsLayout
[self setNeedsStatusBarAppearanceUpdate];
```
