---
title: 新建工程移除StroyBoard
---

1. 移除 `Main.storyboard` 文件
2. 移除 `LaunchScreen.storyboard`
3. 在 `TARGETS` 中，将 `Main InInterface` 选项中的值清空
4. `AppDelegate.m` 替换以下函数，并 `#import "ViewController.h"`

```objc
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions 
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

    ViewController *viewController = [[ViewController alloc] init];
    self.window.rootViewController = viewController;

    self.window.backgroundColor = [UIColor whiteColor];
    [self.window makeKeyAndVisible];

    return YES;
}
```
