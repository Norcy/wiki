本文记录启动优化的过程使用到的一些工具，以及一些小技巧

# 工具
## 测量 Pre-main Time
Xcode 添加环境变量 `DYLD_PRINT_STATISTICS`，设置为 YES，可以在控制台看到

```
Total pre-main time: 667。64 milliseconds (100。0%)
         dylib loading time:  94。93 milliseconds (14。2%)
        rebase/binding time: 117。28 milliseconds (17。5%)
            ObjC setup time: 184。11 milliseconds (27。5%)
           initializer time: 271。21 milliseconds (40。6%)
           slowest intializers :
             libSystem。B。dylib :   7。53 milliseconds (1。1%)
          libglInterpose。dylib : 164。03 milliseconds (24。5%)
         libMTLInterpose。dylib :  23。01 milliseconds (3。4%)
               live4iphone dev :  72。67 milliseconds (10。8%)
```
   
+ 对动态库加载的时间优化：建议减少在App里开发者的动态库集成或者有可能地将其多个动态库最终集成一个动态库后进行导入

+ 减少Appp的Objective-C类，分类和的唯一Selector的个数。这样做主要是为了加快程序的整个动态链接，在进行动态库的重定位和绑定(Rebase/binding)过程中减少指针修正的使用，加快程序机器码的生成

+ 减少Objc运行初始化的时间花费。主要是类的注册，分类的注册，唯一选择器的存在，以及涉及子父类内存布局的Non Fragile ivars偏移的更新，都会影响Objective-C运行时初始化的时间消耗

+ 使用initialize方法进行必要的初始化工作。用+initialize方法替换调用原先在OC的+load方法中执行初始代码工作，从而加快所有类文件的加载速度
            
## Time Profile
使用 Time Profile 检查 App 运行期间各个函数的耗时

记得勾选 Hide System Libraries，这很有用，因为我们只关心自己代码的耗时，不需要关心系统代码的耗时

## FUI
App 的启动时间跟工程中类的个数有关，因此可以使用工具——[Find unused Objective-C imports](https://github。com/dblock/fui) 查找工程中未被 import 的类，删除这些冗余的文件，减少 app 启动时间

删除的时候谨记一点，没有被工程显式 import 的类就一定是没用的吗？注意有些类可能是在运行时通过 runtime 动态生成，别误删了

# 技巧
## 图片合成
针对 UIImageView 连续帧动画场景，原来的方案是每一帧提供一张图片，假如动画有30帧就是30张图片，加载动画的时候就需要30次 io 和文件读取，不利于性能优化，可以考虑将所有帧按顺序排列到一张大图中，加载动画只需读取一张图，每一帧的动画图片可以代码实现大图中截取对应的帧

读取方法如下：

```objc
NSString *imagePath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"LoadingImgs。png"];
UIImage *wholeImg = [UIImage imageWithContentsOfFile:imagePath];
for (int i = 0; i < 3; i++)
{
    for (int j = 0; j < 10; j++)
    {
        CGImageRef imageRef = CGImageCreateWithImageInRect([wholeImg CGImage]， CGRectMake(j*kImgWidth， i*kImgHeight， kImgWidth， kImgHeight);
        if (imageRef)
        {
            [imgs addObject:[UIImage imageWithCGImage:imageRef scale:2。0f orientation:UIImageOrientationUp]];
        }
    }
}
```

## imageWithName v.s. imageWithContentsOfFile
如果图片较大，且只需要用到一次，则应该使用 imageWithContentsOfFile 而不是 imageWithName，更快且省内存，原理参见[《IOS如何选择图片加载方式：imageNamed和imageWithContentsOfFile的区别》](http://blog。csdn。net/wzzvictory/article/details/9053813)，比如上面提到的，Tabbar 上的 icon 合成图，Loading 合成图等等

另外如果图片是 login@2x。png 和 login@3x。png，那么使用 imageWithContentsOfFile 读取时注意，
如果使用以下方法是读取不到图片的

```objc
NSString *imagePath = [[NSBundle mainBundle] pathForResource:@"login" ofType:@"png"];
image = [UIImage imageWithContentsOfFile:imagePath];
```
应该使用

```objc
NSString *imagePath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"login。png"];
image = [UIImage imageWithContentsOfFile:imagePath];
```

详见[《imageWithContentsOfFile遇到的坑》](http://www。jianshu。com/p/5844881b0557)

## load v.s. initialize
尽量别在 load 方法里面写代码，需要初始化全局 oc 对象的话请在 initialize 中完成

|           | load  | initialize || :--: |:--:| :-:|| 执行次数   | 1次 | 1次 || 执行时机   | app启动时所有运行时需要用到的类 | 惰性调用，需要使用到具体类的时候才调用 || 作用      | 调试 现基本不用 |初始化全局oc对象（普通对象可以在声明的时候初始化）|| 执行时环境 | 系统不稳定，许多东西尚未初始化 | 系统处于正常状态 || 调用顺序   | 1. 先调用本类的load，再调用其分类（如果有的话）2. 本类没写 系统不会调用其父类 | 跟其它方法一样 本类没写 会自动调用父类，所以需要先判断类的类名 || 相同点    | 1. 调用的时候其它类不一定准备好 2. 代码要精简，只初始化变量，不调用方法 3. 线程安全，不必加锁| 同左 |


## C 代替 Objective-C 
工程中有一个进行[RC4 加密](https://zh.wikipedia.org/zh-cn/RC4)的函数，其调用次数非常频繁，优化方法是将 oc 对象替换为 c 元素，即将 NSMutableArray 替换为 c 数组，addObject or replaceObjectAtIndex 都替换为对数组元素赋值，objectAtIndex 替换为数组取值，在 iPhone5 上，其单次调用提升了 11 ms

## 网络回包异步线程处理

## 子线程异步上报
之前都是强制在主线程上报，现在改为在子线程异步上报，优化 App 整体速度。注意线程安全问题

## 彩蛋配置
之前 App 启动时都需要加载彩蛋配置（涉及网络环境、App 配置等），现在优化为，如果用户曾经打开过彩蛋，才在启动的时候加载，对于大部分用户来说是不会打开彩蛋的，因此大部分场景可以加快启动速度