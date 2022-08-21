## 现象
1. A.js 部署在 `http://192.168.2.103:5173/`，服务器部署在 `http://192.168.2.103:8888/guess.php`
2. A.js 向 Guess.php 发起 Get 请求时，会报错

```js
Access to fetch at 'http://192.168.2.103:8888/guess.php?reset=1' from origin 'http://192.168.2.103:5173' has been blocked by CORS policy: No 'Access-Control-Allow-Origin' header is present on the requested resource. If an opaque response serves your needs, set the request's mode to 'no-cors' to fetch the resource with CORS disabled.
:8888/guess.php?reset=1:1          
```

## 解决方法（只针对 Get 请求）
注意 fetch 方法中添加 `mode:no-cors` 不可行

只是 Get 请求，可以用 JSONP 的方式来解决

### 客户端的改造
A.js 安装 `fetch-jsonp`，代替 fetch

Before

```js
const response = await fetch('http://192.168.2.103:8888/guess.php?reset=1');
const json = await response.json();
console.log(json)
```

After

```js
import fetchJSONP from 'fetch-jsonp';
const response = await fetchJSONP('http://192.168.2.103:8888/guess.php?reset=1');
const json = await response.json();
console.log(json)
```

### 服务端的改造
Before

```php
$ret = ['state' => 0];
$tmp = json_encode($ret);
echo $tmp;
```

After

```php
$ret = ['state' => 0];
$tmp = json_encode($ret);
$callback = isset($_GET['callback']) ? trim($_GET['callback']) : '';
echo $callback . '(' . $tmp .')';  
```