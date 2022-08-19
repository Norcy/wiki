## MyApp.app
1. 说明

	存放资源文件和可执行文件。整个目录只可读，不可更改。为了防止篡改里面的内容，应用在安装的时候会将该目录进行签名。在非越狱的情况下，该目录中的内容是无法更改的。在越狱设备上如果更改了目录内容，对应的签名就会被改变，这种情况下应用程序将无法启动。

2. iTunes同步

	该文件不会被 iTunes 同步

3. 路径获取

	```objc
	[[NSBundle mainBundle] bundlePath]
	// Output is: /var/containers/Bundle/Application/556BDC9D-1881-48DC-BA34-9A5032621795/MyApp.app"
	```
	
	注意这个 MyApp.app 不是在沙盒的主目录下
	
## 沙盒主目录路径
1. 沙盒主目录的文件结构图如下

		.
		├── Documents
		├── Library
		│   ├── Caches
		│   └── Preferences
		├── SystemData
		└── tmp


2. 路径获取

	```objc
	NSHomeDirectory()
	// Output is: /var/mobile/Containers/Data/Application/E7CA106D-FFFC-47A8-9885-7F6880913E85
	```
	
## Documents
1. 说明

	开发者可以将应用程序的数据文件保存在这个目录下

2. iTunes 同步

	该目录下的文件会被 iTunes 同步。

3. 路径获取

	```objc
	[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0]
	// Output is: /var/mobile/Containers/Data/Application/E7CA106D-FFFC-47A8-9885-7F6880913E85/Documents
	```
 

## Library
1. 说明

	存放一些偏好设置

2. iTunes 同步

	除这个 Library/Caches 之外，Library 下的其他文件会被 iTunes 同步

3. 路径获取

	```objc
	[NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) lastObject]
	// Output is: /var/mobile/Containers/Data/Application/E7CA106D-FFFC-47A8-9885-7F6880913E85/Library
	```
 

## Library/Caches
1. 说明

	存放缓存数据

2. iTunes 同步

	该目录下的数据不会被 iTunes 同步

3. 路径获取
	
	```objc
	[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0]
	// Output is: /var/mobile/Containers/Data/Application/E7CA106D-FFFC-47A8-9885-7F6880913E85/Library/Caches
	```
 

## Library/Preferences
1. 说明

	存放偏好设置文件。比如 NSUserDefaults 就是将数据保存在该目录下的一个plist文件中

2. iTunes 同步

	该目录下的数据会被 iTunes 同步

3. 路径获取

	```objc
	[NSUserDefaults standardUserDefaults]
	// Output is: /var/mobile/Containers/Data/Application/E7CA106D-FFFC-47A8-9885-7F6880913E85/Library/Preferences
	```
 

## Temp
1. 说明

	临时文件，保存应用再次启动时就可以不需要的文件数据。并且开发者不需要这些文件的时候应该要主动将其删除掉，因为该目录下的文件随时可能被系统清理掉，比如当系统磁盘存储空间不足的时候，系统会自动清除这个目录下的文件。

2. iTunes 同步

	该目录不会被 iTunes 同步

3. 路径获取

	```objc
	NSTemporaryDirectory()
	// Output is : /private/var/mobile/Containers/Data/Application/E7CA106D-FFFC-47A8-9885-7F6880913E85/tmp/
	```
