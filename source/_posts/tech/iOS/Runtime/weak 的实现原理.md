## 本文解决的问题
+ weak 的代码实现原理
+ 当对象释放的时候，如何实现将 weak 指针置为 nil
+ weak 指针的线程安全

## 阅读本文的前提
```objc
id __weak weakObj = obj;
```

+ obj 在本文中称之为【被引用对象】，即 referent
+ weakObj 在本文中称之为【弱引用对象】，即 referrer

## weak 的代码实现原理

以上例子中的代码的 Clang 实现如下

```cpp
id weakObj;
objc_initWeak(&weakObj, obj);
```

`objc_initWeak` 源码

```cpp
id objc_initWeak(id *addr, id val) {
    *addr = 0; // 初始化 weakObj = nil
    if (!val) return nil;   // 判空
    return objc_storeWeak(addr, val);
}
```

简化版 `objc_storeWeak` 源码

```objc
id objc_storeWeak(id *location, id newObj) {
    // 获取 oldObj(这个例子中应为 nil)
    id oldObj = *location;

    // 获取 oldObj 对应的 SideTable
    SideTable *oldTable = SideTable::tableForPointer(oldObj);
    // 获取 newObj 对应的 SideTable
    SideTable *newTable = SideTable::tableForPointer(newObj);

    // 从 SideTable 的 weak_table 中移除 oldObj 和它的 weak pointer
    weak_unregister_no_lock(&oldTable->weak_table, oldObj, location);
    // 从 SideTable 的 weak_table 中添加 newObj 和它的 weak pointer
    newObj = weak_register_no_lock(&newTable->weak_table, newObj, location);    
    
    // 给 weak 指针赋值
    *location = newObj;

    // 这个例子中返回值外部没用到
    return newObj;
}
```

相关类的简化版源码

```cpp
// 全局有若干个 SideTable，并不是和 referent 一一对应，多个 referent 可能共享一个 SideTable
class SideTable {
private:
    static uint8_t table_buf[SIDE_TABLE_STRIPE * SIDE_TABLE_SIZE];

public:
    weak_table_t weak_table;
    RefcountMap refcnts;    // 这个是负责引用计数的

    static SideTable *tableForPointer(const void *p) 
    {
        uintptr_t a = (uintptr_t)p;
        int index = ((a >> 4) ^ (a >> 9)) & (SIDE_TABLE_STRIPE - 1);
        return (SideTable *)&table_buf[index * SIDE_TABLE_SIZE];
    }
};
```

`SideTable *oldTable = SideTable::tableForPointer(oldObj);` 

可能看完这句代码，我们会错以为 SideTable 和 obj 是一一对应的，其实并不是

SideTable 有一个成员 `table_buf`，它是 static 的，全局唯一

由 tableForPointer 的源码我们可以知道，只是根据对象的地址来获取 `table_buf` 中的其中一张 SideTable
 
由此可见，SideTable 并不是和 obj 对象一一对应，而是全局有多份，多个对象可能共享同一个 SideTable

另外，可以看到 SideTable 还负责相关对象的引用计数


```cpp
// 全局的弱引用表，与 SideTable 一一对应，Key 是 referent，值是该对象相关的所有弱引用信息（即 weak_entry_t）
struct weak_table_t {
    weak_entry_t *weak_entries; // 所有 referent 对应的 weak_entry_t，这是一个数组
    size_t    num_entries;      // 一共有多少个 referent，即 weak_entries 数组的长度
    ...
};

// 负责维护和存储指向一个对象的所有弱引用 hash 表
// 与 referent 一一对应
struct weak_entry_t {
    DisguisedPtr<objc_object> referent; // 即被引用的对象 obj
    struct {
        weak_referrer_t *referrers;     // 所有弱引用该对象的指针的哈希表
        ...
    }
};

// The address of a __weak object reference
// typedef struct objc_object *id;
// 可以看到 weak_referrer_t 其实就是一个 id 的指针
typedef objc_object ** weak_referrer_t;
```

为了更好的理解 `weak_table_t` 与 obj 之间的对应关系，我们看下 `weak_register_no_lock` 的实现

```cpp
// Adds an (object, weak pointer) pair to the weak table.
id weak_register_no_lock(weak_table_t *weak_table, id referent, id *referrer) {
    ...
    weak_entry_t *entry;
    // weak_entry_for_referent 的实现应该是，遍历 weak_entries 数组，寻找并返回与 referent 对应的 weak_entry_t
    if ((entry = weak_entry_for_referent(weak_table, referent))) {
        // 如果该 referent 已经存在 weak_entry_t（即在此之前已经有弱引用信息），则添加 referrer 到 weak_entry_t
        append_referrer(entry, referrer);
    } 
    else {
        // 如果该 referent 没有 weak_entry_t（即在此之前没有弱引用信息），则新增 weak_entry_t 到 weak_table
        weak_entry_t new_entry(referent, referrer);
        weak_entry_insert(weak_table, &new_entry);
    }
    return referent_id;
}

// Removes an (object, weak pointer) pair from the weak table.
void weak_unregister_no_lock(weak_table_t *weak_table, id referent, id *referrer) {
    if (!referent) return;

    weak_entry_t *entry;
    // weak_entry_for_referent 的实现应该是，遍历 weak_entries 数组，寻找并返回与 referent 对应的 weak_entry_t
    if ((entry = weak_entry_for_referent(weak_table, referent))) {
        // 如果该 referent 已经存在 weak_entry_t（即在此之前已经有弱引用信息），则从 weak_entry_t 中删除 referrer
        // remove_referrer 这个方法会把这个 referrer 置为 nil（这个就是 weak 指针自动变 nil 的原因哦）
        remove_referrer(entry, referrer);
        
        // 视情况从 weak_table 中删除 referent 对应的 weak_entry_t
        ...
        if (empty) {
            weak_entry_remove(weak_table, entry);
        }
    }
    // 如果调用正确，理论上 weak_entries 数组应该会有 referent 的 weak_entry_t 信息
}
```

![关系图.png](http://ww1.sinaimg.cn/large/99e3e31egy1gb5ec55d05j20id0jlq48.jpg)
<!-- ![](http://solacode.github.io/img/Screen%20Shot%202015-10-21%20at%2021.42.02.png) -->


## 当对象释放的时候，如何实现将 weak 指针置为 nil
```objc
// NSObject.m
- (void)dealloc {
    _objc_rootDealloc(self);
}

void _objc_rootDealloc(id obj) {
    obj->rootDealloc();
}

void objc_object::rootDealloc() {
    ...
    object_dispose((id)this);
}

id object_dispose(id obj)  {
    if (!obj) return nil;
    objc_destructInstance(obj);    
    free(obj);
    return nil;
}

void *objc_destructInstance(id obj) {
    if (obj) {
        bool cxx = obj->hasCxxDtor();
        bool assoc = obj->hasAssociatedObjects();
        if (cxx) object_cxxDestruct(obj);
        if (assoc) _object_remove_assocations(obj);
        obj->clearDeallocating();
    }
    return obj;
}

// 没找到 clearDeallocating 源码，但是大致实现如下
void objc_object::clearDeallocating() {
    SideTable *table = SideTable::tableForPointer(this);
    weak_clear_no_lock(&table->weak_table, (id)this);
}

void weak_clear_no_lock(weak_table_t *weak_table, id referent_id) 
{
    objc_object *referent = (objc_object *)referent_id;
    // 从 weak_table 中寻找该 referent 对应的 weak_entry_t
    weak_entry_t *entry = weak_entry_for_referent(weak_table, referent);
    weak_referrer_t *referrers;
    size_t count;
    referrers = entry->referrers;
    count = TABLE_SIZE(entry);
    
    // 清除所有 referrer 的值
    for (size_t i = 0; i < count; ++i) {
        objc_object **referrer = referrers[i];
        if (referrer) {
            // referrer 指向的值，不出意外应该与 referent 相等
            if (*referrer == referent) {
                // 置空，这个就是为什么 weak 会自动置 nil 的原因
                *referrer = nil;
            }
        }
    }
    // 从 weak_table 中删除对应的 weak_entry_t
    weak_entry_remove(weak_table, entry);
}
```

1. 可以看到对象释放时会调用 dealloc，
2. 一步步调用到了 clearDeallocating，然后调用 tableForPointer 寻找对应的 SideTable，拿到 `weak_table_t`
3. 最终调用 `weak_clear_no_lock`，将所有的 referrer 指向的值（即 weak 指针），置为 nil，并从 `weak_table_t` 表中删除该对象的 `weak_entry_t`

通俗解释：

系统会把 weakObj 会放入一个 hash 表中。 用 obj 的内存地址作为 key，当 obj 的引用计数为 0 的时候会执行其 dealloc，此时会在这个 weak 表中搜索，找到所有以 &obj 为 key 的对象，设置为 nil

## weak 指针的线程安全
问题：当一个对象正在 delloc 时，如果在另一个线程获取了 weak 指针，这时获取到的 weak 指针有没有可能是野指针？

以下的代码例子模拟了这样一个过程，多个线程正在访问 weakObj，其中一个线程对 self.obj 释放了

```objc
// @property (nonatomic, strong) NSObject *obj;
self.obj = [NSObject new];
int n = 500;
while (n--)
{
    __weak NSObject *weakObj = self.obj;
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        if (n == 480)
        {
            self.obj = nil;
        }
        int m = 1000;
        while (m--)
        {
            NSLog(@"%@----%@", weakObj, @(n));
        }
    });
}
```

结论：不会挂，不可能是野指针。weak 的访问是线程安全的

```objc
Person *obj = [[Person alloc] init];
id __weak weakObj = obj;
NSLog(@"%@", weakObj);
```

通过 `clang -rewrite-objc MyBlock.c` 重写后得到的伪代码

```objc
id weakObj;
objc_initWeak(&weakObj, obj);

// 注意 NSLog(@"%@", weakObj) 转为以下代码
id tmp = objc_loadWeakRetained(&obj);
NSLog(@"%@", tmp);
objc_release(tmp);
```

当我们访问 weakObj 的时候，编译器会转为 `objc_loadWeakRetained`

```objc
id objc_loadWeakRetained(id *location)
{
    id obj;
    id result;
    Class cls;
    SideTable *table;
    
 retry:
    obj = *location;
    if (!obj) return nil;
    if (obj->isTaggedPointer()) return obj;
    
    table = &SideTables()[obj];
    
    table->lock();
    if (*location != obj) {
        table->unlock();
        goto retry;
    }
    
    result = obj;

    cls = obj->ISA();
    if (! cls->hasCustomRR()) {
        // 一般情况下会走到这里
        if (! obj->rootTryRetain()) {
            result = nil;
        }
    }
    else {
        // 此处省略不重要的代码...
    }
        
    table->unlock();
    return result;
}
```

1. 获取 weak 指针时，会调用 `objc_loadWeakRetained`
2. 不讨论 isTaggedPointer 这种特殊情况
3. hasCustomRR 在重写 retain/release/autorelease/retainCount/_tryRetain/_isDeallocating/retainWeakReference/allowsWeakReference 等方法时会返回true，一般情况会返回 false。这里只讨论返回 false 的情况
4. rootTryRetain 会尝试对该对象进行 retain，里面会判断该对象是否正在 deallocating，如果是则返回 nil
5. 通俗概括以上代码：获取 weak 时调用 `objc_loadWeakRetained`，获取过程会加锁。如果该对象已经释放或正在释放则返回 nil，否则对该对象进行 retain 并返回。因此我们得出结论：对 weak 指针的访问是线程安全的
6. 那么问题来了，既然有 retian，那什么时候 release 呢？答案是 ARC 下会在 weak 指针访问完成后，自动插 release 代码，如下

```objc
// 注意 NSLog(@"%@", weakObj) 转为以下代码
id tmp = objc_loadWeakRetained(&obj);
NSLog(@"%@", tmp);
objc_release(tmp);
```

## 参考
+ [Runtime如何实现weak属性？](http://solacode.github.io/2015/10/21/Runtime%E5%A6%82%E4%BD%95%E5%AE%9E%E7%8E%B0weak%E5%B1%9E%E6%80%A7%EF%BC%9F/)
+ [objc-weak.h 源码](https://opensource.apple.com/source/objc4/objc4-647/runtime/objc-weak.h)
+ [SiteTable 源码](https://opensource.apple.com/source/objc4/objc4-647/runtime/NSObject.mm)
+ [详解获取weak对象的过程](https://www.codenong.com/j5defc55351882512327/)



