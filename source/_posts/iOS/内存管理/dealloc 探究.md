## 结论
1. dealloc 是在最后一次 release 执行后调用
2. ARC 下，对象的实例变量的释放时机是：编译器在 NSObject 插入的 `.cxx_desctruct` 方法释放
3. ARC 下 `[super dealloc]` 方法是由编译器自动插入，所以不需要手动调用
4. dealloc 主要干三件事：
    1. 释放对象的实例变量
    2. 释放对象的关联对象（即释放该类的分类里设置的关联属性）
    3. 将所有指向该对象的 weak 指针置为 nil

## 源码探讨
通过 apple 的 runtime 源码，不难发现 NSObject 执行 dealloc 最后执行到 `objc_destructInstance`

```cpp
void *objc_destructInstance(id obj)
{
    if (obj) {
        Class isa_gen = _object_getClass(obj);
        class_t *isa = newcls(isa_gen);

        // Read all of the flags at once for performance.
        bool cxx = hasCxxStructors(isa);
        bool assoc = !UseGC && _class_instancesHaveAssociatedObjects(isa_gen);

        // 重点
        if (cxx) object_cxxDestruct(obj);
        if (assoc) _object_remove_assocations(obj);

        if (!UseGC) objc_clear_deallocating(obj);
    }

    return obj;
}
```

简单明确的干了三件事：

1. `object_cxxDestruct`：strong 修饰的变量执行 objc_storeStrong(&ivar, nil) release 对象，ivar 赋值 nil；weak 修饰的变量执行 objc_destroyWeak(&ivar) -> storeWeak(&ivar, nil ) -> weak_unregister_no_lock，将变量指向 nil，且删除变量对象的 weak 相关信息（referrers 移除 weak 地址）
2. 执行 `_object_remove_assocations` 去除和这个对象 assocate 的对象（常用于 category 中添加带变量的属性）
3. 执行 `objc_clear_deallocating`，清空引用计数表并清除弱引用表，将所有 weak 引用指 nil（这也就是 weak 变量能安全置空的所在）

## 面试题：引用计数减为 0 会立即释放吗
答案：会。dealloc 是在最后一次 release 执行后调用

## 参考文章
+ [ARC 下 dealloc 过程及.cxx_destruct 的探究](https://blog.sunnyxx.com/2014/04/02/objc_dig_arc_dealloc/)