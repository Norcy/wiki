iOS 中有两种常用的图片加载方式：

## `imageNamed`
1. 有缓存：这种方法会首先在系统缓存中根据指定的名字寻找图片，如果找到了就返回。如果没有在缓存中找到图片，该方法会从指定的文件中加载图片数据，并将其缓存起来，然后再把结果返回

2. 适合使用频率高的小图片

3. 系统会自动添加 `@2x`/`@3x` 去寻找图片

## `imageWithContentsOfFile`
1. 只是简单的加载图片，并不会将图片缓存起来

2. 适合使用频率少的大图片

3. 关于 `@2x`/`@3x` 的坑

	假如现在有 image@2x.png 和 image@3x.png 2张图片
	
	以下代码可以正确找到图片，这种方法是直接寻找 image@2x.png
	
	```objc
	NSString *path = [[NSBundle mainBundle] pathForResource:@"image@2x" ofType:@"png"]; 
	UIImage *image = [UIImage imageWithContentsOfFile:path];
	```
	
	以下代码无法找到图片，这种方法是直接寻找 image.png
	
	```objc
	NSString *path = [[NSBundle mainBundle] pathForResource:@"image" ofType:@"png"]; 
	UIImage *image = [UIImage imageWithContentsOfFile:path];
	```
	
	以下代码可以正确找到图片，这种方法系统会帮助处理 `@2x`/`@3x`
	
	```objc
	NSString *path = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"image.png"];
	UIImage *image = [UIImage imageWithContentsOfFile:path];
	```

## Framework 的图片加载
提供 Framework 提供给业务方时，需要提供 .bundle 和 .framework

主 App 将 .bundle 放入自己的 bundle 中

Framework 使用以下代码访问图片资源

```objc
NSBundle *bundle = [NSBundle bundleWithPath:[[NSBundle mainBundle] pathForResource:@"VideoNativeFramework" ofType:@"bundle"]];
UIImage *img = [UIImage imageWithContentsOfFile:[[bundle resourcePath] stringByAppendingPathComponent:@"image.png"]];
```