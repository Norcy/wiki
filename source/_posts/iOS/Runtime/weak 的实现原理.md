## 本文解决的问题
+ weak 的代码实现原理
+ 当对象释放的时候，如何实现将 weak 指针置为 nil

## 阅读本文的前提
```objc
id __weak weakObj = obj;
```

+ 被引用对象：obj
+ weak 变量：weakObj

## weak 的代码实现原理


## 当对象释放的时候，如何实现将 weak 指针置为 nil

![](http://solacode.github.io/img/Screen%20Shot%202015-10-21%20at%2021.42.02.png)


## 参考
+ [Runtime如何实现weak属性？](http://solacode.github.io/2015/10/21/Runtime%E5%A6%82%E4%BD%95%E5%AE%9E%E7%8E%B0weak%E5%B1%9E%E6%80%A7%EF%BC%9F/)

## 待整理
1. 什么 weak 属性

    weak 表示的是一个弱引用，不会增加对象的引用计数，并且在所指向的对象被释放之后，weak 指针会被设置的为 nil



[Runtime如何实现weak属性？](http://solacode.github.io/2015/10/21/Runtime%E5%A6%82%E4%BD%95%E5%AE%9E%E7%8E%B0weak%E5%B1%9E%E6%80%A7%EF%BC%9F/)
[源码](https://opensource.apple.com/source/objc4/objc4-647/runtime/NSObject.mm)


```objc
__wead id weakObj = obj;
```

关键要理解好 weakObj 和 obj 的关系，obj 是被指向的对象，weakObj 是被 weak 修饰的对象

系统会把 weakObj 会放入一个 hash 表中。 用 obj 的内存地址作为 key，当 obj 的引用计数为 0 的时候会执行其 dealloc，此时会在这个 weak 表中搜索，找到所有以 &obj 为 key 的对象，设置为 nil

被简化了的源码，暂时不看

```objc
id objc_storeWeak(id *location, id newObj)
{
    id oldObj;
    SideTable *oldTable;
    SideTable *newTable;
    
    oldObj = *location;

    oldTable = SideTable::tableForPointer(oldObj);
    newTable = SideTable::tableForPointer(newObj);

    weak_unregister_no_lock(&oldTable->weak_table, oldObj, location);
    newObj = weak_register_no_lock(&newTable->weak_table, newObj, location);
    *location = newObj;

    return newObj;
}
```
