## == 和 === 区别


> `===` 严格相等，会比较两个值的类型和值
> 
> `==`  抽象相等，比较时，会先进行类型转换，然后再比较值

建议用 `===`

具体可见

+ [js 中 == 和 === 的区别](https://juejin.im/entry/584918612f301e005716add6)
+ [JavaScript 中的相等性判断
](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Equality_comparisons_and_sameness)

## Js 中如何正确创建一个类

```js
function Cat(name,color){
　　this.name = name;
　　this.color = color;
}

Cat.prototype.type = "猫科动物";
Cat.prototype.eat = function(){alert("吃老鼠")};
```

1. 使用构造函数，成员变量使用 this 在构造函数中初始化；

	> 所谓"构造函数"，其实就是一个普通函数，但是内部使用了this变量。对构造函数使用new运算符，就能生成实例，并且this变量会绑定在实例对象上。

	```js
	var cat1 = new Cat("大毛","黄色");
	var cat2 = new Cat("二毛","黑色");
	alert(cat1.name); 	// 大毛
	alert(cat1.color); 	// 黄色
	```
	

2. 类变量和方法定义在 prototype 上

	构造函数有一个 prototype 对象，该对象的所有属性和方法，都会被构造函数的实例继承。因此在这里定义类变量和方法可以实现不同对象之间的数据共享

+ 参考：[Javascript 面向对象编程（一）：封装](http://www.ruanyifeng.com/blog/2010/05/object-oriented_javascript_encapsulation.html)



## JS 数组深拷贝
```js
const newArray = JSON.parse(JSON.stringify(array));
```


## JS Fetch 请求使用 `application/x-www-form-urlencoded` 的坑
body 参数必须自己手动编码，否则会出错

```js
var details = {
    'userName': 'test@gmail.com',
    'password': 'Password!',
    'grant_type': 'password'
};

var formBody = [];
for (var property in details) {
  var encodedKey = encodeURIComponent(property);
  var encodedValue = encodeURIComponent(details[property]);
  formBody.push(encodedKey + "=" + encodedValue);
}
formBody = formBody.join("&");

fetch('https://example.com/login', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8'
  },
  body: formBody
})
```