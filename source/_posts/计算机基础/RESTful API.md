## 什么是 REST
REST：REpresentational State Transfer，直译就是 “表现层状态转移”

通俗地讲就是，URL 定位资源，用 HTTP 动词（GET,POST,PUT,DELETE）描述操作

+ Resource：资源，即数据
+ Representational：某种表现形式，比如用 JSON，XML，JPEG 等
+ State Transfer：状态变化。通过 HTTP 动词实现

REST 本身不实用，实用的是如何设计 RESTful API

## 什么是 RESTful API
RESTful API 就是 REST 风格的 API，它使用一套协议来规范多种形式的前端和同一个后台的交互方式

RESTful 是基于 HTTP 协议，设计原则如下：

1. 对资源的操作。资源就是一段文本、一张图片或一段视频。资源总是要通过一种载体来反应它的内容，比如文本用 txt，图片用 png 或 jpg，而 JSON 是现在最常用的资源表现形式

2. 使用 HTTP 动词来实现增删改查。RESTful API 对资源的 CRUD（create,read,update,delete）分别对应 HTTP 的方法：GET 用来获取资源，POST 用来新建资源（也可以用于更新资源），PUT 用来更新资源，DELETE 用来删除资源

3. 使用 URI（统一资源定位符）。每个 URI 都对应一个特定的资源，最典型的 URI 就是 URL。每个网址代表一种资源，所以网址中不能有动词，只能有名词

4. 无状态。所有的资源都可以 URI 定位，而且这个定位与其他资源无关，也不会因为其他资源的变化而变化。比如查询工资的接口必须在员工请求登录接口之后才能查询，它就不属于无状态的


其他规范：

+ 将 API 的版本号放入 URL，比如 `https://api.example.com/v1/`
+ 一般 API 中的名字都使用复数

	```
	https://api.example.com/v1/zoos
	https://api.example.com/v1/animals
	https://api.example.com/v1/posts
	```

## For Example
GET:`http://api.example.com/posts/123` 表示获取 ID 为 123 的帖子

GET:`http://api.example.com/posts` 表示获取所有帖子列表

DELETE:`http://api.example.com/posts/123` 则为对应的删除操作

错误的示例：`http://api.example.com/getPosts/123` or `http://api.example.com/getPosts/123`，这里网址中有了动词，并不是通过 HTTP 动词来实现增删改查


## 参考文章
+ [《什么是 RESTful API？》](https://blog.csdn.net/hjc1984117/article/details/77334616)
+ [《RESTful API 设计指南》](http://www.ruanyifeng.com/blog/2014/05/restful_api.html)