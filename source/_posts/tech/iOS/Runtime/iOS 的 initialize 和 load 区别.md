|           | load  | initialize |
| :--: |:--:| :-:|
| 执行次数   | 1次 | 每个类会被系统只调用一次，但是由于继承的关系，子类未实现的情况下父类的方法会被多次调用 |
| 执行时机   | 所有运行时需要用到的类<br>在main函数开始执行之前，与这个类是否被用到无关 | 懒加载，需要使用到具体类的时候才调用 <br>（类或它的子类收到第一条消息之前被调用的，包括实例方法和类方法，如果没收到消息则永远不会调用） |
| 作用      | Runtime 交换方法 | 初始化全局 oc 对象 <br>（普通对象可以在声明的时候初始化）|
| 执行时环境 | 系统不稳定，许多东西尚未初始化，调用的时候其它类不一定准备好 | 系统处于正常状态，其他类的方法都能正常调用 <br> （特殊情况：如果在 load 方法调用该类的方法会导致 initalize 提前调用，这种情况系统并不稳定） |
| 调用顺序   | 1. 顺序：父类->本类->分类 <br> 2. 分类不会影响本类，分类之间的 load 顺序无法确定 <br> 3. 本类没写，系统在加载本类的时候不会调用其父类 <br> 4. 不同的类或者分类之间的顺序是按 Compile Sources 来确定 | 1. 顺序：父类->本类->分类 <br> 2. 分类的实现会覆盖本类 <br> 3. 本类没写，会自动调用父类，所以需要先判断类名 |
| 相同点    |  1. 代码要精简，避免处理复杂逻辑 <br> 2 线程安全，不必加锁 <br> 3. 开发者不能显式调用，也不能调用 super <br> 4. 最好不要调用其他类的方法 | 同左 |


> + load 中最好不要调用其他类的方法，是因为调用的时候其他类不一定加载好
> + initialize 中最好不要调用其他类的方法，是因为如果这样做，可能产生循环依赖，比如 A 的 initialize 调用了 B 的方法，导致 B 的 initialize 被调用，而 B 的 initialize 也调用了 A 的方法，此时会有问题（而现实情况的互相依赖可能涉及多个类，一旦出现问题就难以定位）

# load
## 父类的 load 优先于子类
```objc
static void schedule_class_load(Class cls)
{
    if (!cls) return;
    assert(cls->isRealized());  // _read_images should realize

    if (cls->data()->flags & RW_LOADED) return;

    // Ensure superclass-first ordering
    schedule_class_load(cls->superclass);

    add_class_to_loadable_list(cls);
    cls->setInfo(RW_LOADED);
}
```

可以看到，对 cls 的处理过程中，优先递归处理了父类，因此父类的 load 一定比子类的优先调用

## 本类的 load 优先于分类
```objc
void call_load_methods(void)
{
    static BOOL loading = NO;
    BOOL more_categories;

    recursive_mutex_assert_locked(&loadMethodLock);

    // Re-entrant calls do nothing; the outermost call will finish the job.
    if (loading) return;
    loading = YES;

    void *pool = objc_autoreleasePoolPush();

    do {
        // 1. Repeatedly call class +loads until there aren't any more
        while (loadable_classes_used > 0) {
            call_class_loads();
        }

        // 2. Call category +loads ONCE
        more_categories = call_category_loads();

        // 3. Run more +loads if there are classes OR more untried categories
    } while (loadable_classes_used > 0  ||  more_categories);

    objc_autoreleasePoolPop(pool);

    loading = NO;
}
```

其中 `call_class_loads` 是调用本类的 load，而 `call_category_loads` 是调用分类的 load


## 系统加载本类的时候为什么不会调用父类
这里有一点值得探讨：**本类没写，系统在加载本类的时候不会调用其父类**

探讨这个问题的前提是，父类的 load 方法是存在的，假如没有实现，讨论调用时机是没有意义的

这里可能会误解为，本类没写，其父类的 load 也不会被调用。加载子类的时候，如果子类没有实现 load 方法，那么系统是不会在此时自动调用父类的 load，因为 load 调用是直接获取函数指针来执行，不会像 `objc_msgSend` 一样会有方法查找的过程，也就不会沿着继承链往上寻找了。但是这个不意味着父类的 load 方法不会被调用，因为父类也是需要被加载的，所以 load 方法也会被调用，所以这里强调的是时机，本类没有写 load 的情况下，**系统加载本类的时候**不会调用其父类的 load

```objc
static void call_class_loads(void)
{
    int i;

    // Detach current loadable list.
    struct loadable_class *classes = loadable_classes;
    int used = loadable_classes_used;
    loadable_classes = nil;
    loadable_classes_allocated = 0;
    loadable_classes_used = 0;

    // Call all +loads for the detached list.
    for (i = 0; i < used; i++) 
    {
        Class cls = classes[i].cls;
        // 这里取到的就是 load 方法
        load_method_t load_method = (load_method_t)classes[i].method;
        if (!cls) continue;
        (*load_method)(cls, SEL_load);
    }

    // Destroy the detached list.
    if (classes) _free_internal(classes);
}
```

`(*load_method)(cls, SEL_load);` 说明，load 方法不是通过 `objc_msgSend` 调用，而是直接通过函数指针调用，因此不会在此次调用中调用父类实现


# initialize
## initialize 的正确写法（重要！！！不然有子类的情况下可能会调用多次）
假设我想要在 A 的 initialize 方法中打印出自己，如果这样写：

```objc
@interface A : NSObject
@end
@implementation A
+ (void)initialize
{
    NSLog(@"%@", self);
}
@end

@interface B : A
@end
@implementation B
@end
```

此时创建一个B对象，输出是
> A  
> B

初始化 B 的时候，要先初始化 A，所以输出 A，然后初始化 B，由于 B 没有实现 initialize，所以系统调用了 A 的方法，此时 self 是 B

所以 A 的 initialize 方法应该这样写

```objc
+ (void)initialize
{
    if (self == [A class])
    {
        NSLog(@"%@", self);
    }
}
```
此时创建一个B对象，输出是
> B

## initialize 的调用顺序

以下关键代码来自 objc-runtime-new.mm，当我们给某个类发送消息时，Runtime 会调用该函数。当类没有初始化会调用 `void _class_initialize(Class cls)` 对该类进行初始化

```objc
IMP lookUpImpOrForward(Class cls, SEL sel, id inst,
    bool initialize, bool cache, bool resolver)
{
    // 省略部分代码
    if (initialize && !cls->isInitialized())
    {
        // 省略部分代码
        _class_initialize(_class_getNonMetaClass(cls, inst));
        // 省略部分代码
    }
    // 省略部分代码
}
```

`_class_initialize` 关键代码如下

1. 可以看到优先递归处理了父类，因此**父类的 initialize 一定优先于子类**
2. 除此之外，从 `((void (*)(Class, SEL))objc_msgSend)(cls, SEL_initialize);` 看到，本类的 initialize 是通过 `objc_msgSend` 进行调用，与普通方法的调用是一样的，**如果子类没有实现，那么父类的实现会被调用**；**如果一个类的分类实现了 initialize 方法，那么就会对这个类中的实现造成覆盖**

```objc
void _class_initialize(Class cls)
{
    // 省略部分代码
    Class supercls;
    BOOL reallyInitialize = NO;
    supercls = cls->superclass;
    if (supercls && !supercls->isInitialized())
    {
        _class_initialize(supercls);
    }

    if (!cls->isInitialized() && !cls->isInitializing())
    {
        cls->setInitializing();
        reallyInitialize = YES;
    }

    if (reallyInitialize)
    {
        // 省略部分代码
        ((void (*)(Class, SEL))objc_msgSend)(cls, SEL_initialize);
        // 省略部分代码
    }
    // 省略部分代码
}
```

# 题目实战
Compile Sources 中有以下类，顺序如下

```
Daughter.m
Other.m
Father+Category_2.m
Son+Category.m
Son.m
main.m
Other+Category.m
Father+Category_1.m
Father.m
Appdelegate.m
```

顾名思义，Father 有是 Son 和 Daughter 的父类，Other 与他们没有继承关系

其中除了 Daughter，其他类都写了 load 方法和 initialize 方法，如下

```objc
+ (void)load
{
    NSLog(@"%s %@", __FUNCTION__, [self class]);
}

+ (void)initialize
{
    NSLog(@"%s %@", __FUNCTION__, [self class]);
}

// main.m
int main(int argc, char * argv[]) 
{
    @autoreleasepool 
    {
        NSLog(@"Main 函数开始执行");
        Daughter *daugter = [[Daughter alloc] init];
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
```

输出结果如下：

```
+[Other(Category) initialize] Other
+[Other load] Other
+[Father(Category_1) initialize] Father
+[Father load] Father
+[Son(Category) initialize] Son
+[Son load] Son
+[Father(Category_2) load] Father
+[Son(Category) load] Son
+[Other(Category) load] Other
+[Father(Category_1) load] Father
Main 函数开始执行
+[Father(Category_1) initialize] Daughter
```

1. **类的加载顺序由 Compile Sources 确定**，即 Daughter > Other > Son > Father，分类的加载是另外一个时机
2. 第一个加载的类是 Daughter，由于没有实现 load，所以没有任何关于 Daughter 的输出，同时也可以看出，**一个类没有实现 load，加载它时不会调用父类的 load**
343. 第二个加载的类是 Other，由于在其 load 的中向本类发送了消息，所以导致 initialzie 先于 load 被调用，可以看出，**initialize 调用的时机不是在 main 函数之后，而是在向该类发送第一个消息之前**；第一行输出是 Other 分类的 initialize 方法， Other 自己的 intialize 永远没机会被调用，是因为**分类的 innitialize 会覆盖本类**
4. 第三个加载的类是 Son，加载 Son 的时候会优先加载 Father，说明**子类的 load 一定是晚于父类**；同 Other，Father 优先输出了分类的 initialize，再输出 load，那为什么是 `Category_1` 而不是 `Category_2` 的 initialze 呢，因为 `Category_2` 在 Compile Sources 中是晚于 `Category_1`，因此覆盖了本类和 `Category_1` 的方法。即**分类的 initialzie 覆盖本类时，以 Compile Sources 中最后一个分类为准**
5. 接下来终于轮到 Son 自己加载了，可以看到 Son+Category 的 load 的方法并没有在此时执行，说明**分类的 load 确实晚于主类**
6. 最后一个要加载的主类是 Father，而由于 Father 刚刚已经加载过了，因此不会再次调用 load 方法，说明**一个类的 load 方法系统最多只会调用一次**
7. 主类加载完之后，终于轮到分类了，**分类的加载顺序取决于其在 Compile Sources 的顺序，而与它们的主类在 Complie Sources 的顺序无关**，本例中 `Father(Category_2)` > Son(Category) > Other(Category) > `Father(Category_1)`
8. 主类分类加载完毕，开始执行 main 函数。说明**类的 load 都是在 main 函数之前**
9. 最后在 main 函数中向 Daughter 发送了一个消息，从而触发了 Daughter 的 initialize，可以看出 **initialize 是懒加载的**；其次，**本类没实现 initialize 的时候，系统会自动调用父类的实现**


# 延伸思考
## 为什么分类的 load 方法不会被覆盖本类，而 innitialize 会呢
因为 load 的时候 Runtime 还没有初始化完毕；load 的调用是直接函数调用，而 initialize 是属于消息发送，需要依赖 Runtime

## 分类实现的普通方法，是如何覆盖本类的


## 为什么方法交换要写在 load
我们可以从 load 的特点得到：

+ 方法交换调用一次就够了，而 load 只会被系统调用一次
+ 方法交换越早越好，而 load 方法在 main 函数之前就被调用了

但是有这两个原因仍然不足以说明为什么要写在 load，因为 initialize 方法也可以做到一次和越早越好

那么 load 为什么比 initialize 更适合呢，答案是不会有被覆盖的风险，写了就一定会被调用到

写在主类的 initialize 的方法可能被其分类的 initialize 覆盖，而 load 不会
	
## 为什么方法交换需要加 dispatch_once，不是说 load 只会执行一次吗
load 是线程安全的，最多也被系统调用一次，添加 `dispatch_once` 完全是为了防止不合格的程序员手动调用

## 多次交换会有什么问题
```objc
// UIViewController+Category1.m
+ (void)load
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        swizzleMethod(self, @selector(viewWillAppear), @selector(viewWillAppear_1));
    });
}

- (void)viewWillAppear_1:(BOOL)animated
{
    NSLog(@"%s", __FUNCTION__);
    [self viewWillAppear_1:animated];
}

// UIViewController+Category2.m
+ (void)load
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        swizzleMethod(self, @selector(viewWillAppear), @selector(viewWillAppear_2));
    });
}

- (void)viewWillAppear_2:(BOOL)animated
{
    NSLog(@"%s", __FUNCTION__);
    [self viewWillAppear_2:animated];
}
```

假如 UIViewController+Category1 的 load 比 UIViewController+Category2 先执行，那么执行此时所有的 UIViewController 即将出现的时候将会调用哪些方法

答案：调用顺序如下

```objc
viewWillAppear_2
viewWillAppear_1
viewWillAppear
```

交换之前

```objc
@selector(viewWillAppear) -> viewWillAppear's IMP
@selector(viewWillAppear_1) -> viewWillAppear_1's IMP
@selector(viewWillAppear_2) -> viewWillAppear_2's IMP
```

第一次交换：UIViewController+Category1 的 load

```objc
@selector(viewWillAppear) -> viewWillAppear_1's IMP
@selector(viewWillAppear_1) -> viewWillAppear's IMP
@selector(viewWillAppear_2) -> viewWillAppear_2's IMP
```

第二次交换：UIViewController+Category2 的 load

```objc
@selector(viewWillAppear) -> viewWillAppear_2's IMP
@selector(viewWillAppear_1) -> viewWillAppear's IMP
@selector(viewWillAppear_2) -> viewWillAppear_1's IMP
```

1. 调用 viewWillAppear，执行的是其对应的 IMP， 即 `viewWillAppear_2`
2. 然后 `viewWillAppear_2` 的 IMP 里又调用了 @selector(viewWillAppear_2) ，执行的是 `viewWillAppear_1` 的 IMP
3. 最后 `viewWillAppear_1` 的 IMP 里又调用了 @selector(viewWillAppear_1) ，执行的是 `viewWillAppear` 的 IMP


## load 方法中实现方法交换的时候采用 c 函数
swizzleMethod 采用 C 函数，而不是 NSObject 的方法，是为了防止子类在 load 方法中向其自己发送消息，那样会导致其 +initialize 方法在 load 的时候被提前调用，而此时系统环境是不稳定的

# 参考文章
+ [《Objective-C +load vs +initialize》](http://blog.leichunfeng.com/blog/2015/05/02/objective-c-plus-load-vs-plus-initialize/)
+ [《一道题搞清楚Objective-C中load和initialize》](https://www.jianshu.com/p/ffdefa76e4a2)
+ 《Effective Objective-C 2.0 编写高质量 iOS 与 OS X 代码的52个有效方法》第 51 条
