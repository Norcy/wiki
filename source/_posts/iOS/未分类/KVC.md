## 基本概念
+ KVC 是 Key-Value Coding 的缩写，让开发者可以通过 Key 直接访问或设置对象的属性/成员变量，而不需要调用存取方法
+ KVC 可以直接获取、修改类不想暴露的私有变量，所以会破坏面向对象的编程思想
+ UITextView 设置 `_placeholderLabel` 可用到

## 基础用法
```objc
- (id)valueForKey:(NSString *)key;                          // 直接通过 Key 来取值
- (void)setValue:(id)value forKey:(NSString *)key;          // 通过 Key 来设值

- (id)valueForKeyPath:(NSString *)keyPath;                  // 通过 KeyPath 来取值
- (void)setValue:(id)value forKeyPath:(NSString *)keyPath;  // 通过 KeyPath 来设值
```

通过 Key 来访问直接的属性

```objc
// age 是 b 的属性
[b setValue:@23 forKey:@"age"];
```

通过 KeyPath 来访问属性的属性

```objc
// b 是 a 的一个属性
[a setValue:@23 forKeyPath:@"b.age"];
```

## 实现原理
### KVC setter 的查找过程
以 `[b setValue:@23 forKey:@"age"];` 举例

1. 查找 B 中的 `setAge:`、`_setAge:` 方法；找到则调用，结束
2. 查看 B 的 `+ (BOOL)accessInstanceVariablesDirectly` 返回值，若为 NO 则调用 B 的 `setValue:forUndefinedKey:` 并抛异常，结束（默认值为 YES）
3. 查找 B 中的 `_age`、`_isAge`、`age`、`isAge`，找到则直接访问；否则调用 B 的 `setValue:forUndefinedKey:` 并抛异常


### KVC getter 的查找过程
以 `[b valueForKey:@"age"];` 举例

1. 查找 B 中的 `getAge`、`age`、`isAge`、`_getAge`、`_age`、`_isAge` 方法；找到则调用，结束
2. 查找一些集合类的特有方法，比如 count/sum/average 等，这里不展开
3. 查看 B 的 `+ (BOOL)accessInstanceVariablesDirectly` 返回值，若为 NO 则调用 B 的 `valueForUndefinedKey:` 并抛异常，结束（默认值为 YES）
3. 查找 B 中的 `_age`、`_isAge`、`age`、`isAge`，找到则直接访问；否则调用 B 的 `valueForUndefinedKey:` 并抛异常

> accessInstanceVariablesDirectly 方法，顾名思义，是否允许直接访问成员变量，默认 YES
