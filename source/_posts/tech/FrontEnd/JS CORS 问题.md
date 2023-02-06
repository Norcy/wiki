## 现象
1. A.js 部署在 `http://192.168.2.103:5173/`，服务器部署在 `http://192.168.2.103:8888/guess.php`
2. A.js 向 Guess.php 发起 Get 请求时，会报错

```js
Access to fetch at 'http://192.168.2.103:8888/guess.php?reset=1' from origin 'http://192.168.2.103:5173' has been blocked by CORS policy: No 'Access-Control-Allow-Origin' header is present on the requested resource. If an opaque response serves your needs, set the request's mode to 'no-cors' to fetch the resource with CORS disabled.
:8888/guess.php?reset=1:1          
```
## 解决方法 1：前后端配合

+ JS 的 fetch header 的 content-type 设置为 'application/x-www-form-urlencoded; charset=UTF-8' 即可，而不能是其他的
+ 后端设置 `Access-Control-Allow-Origin` 为 `*` 或指定 url

```js
const obj = {a:1};
const options = {
    method: "POST",
    headers: {
    "Content-Type": "application/x-www-form-urlencoded; charset=UTF-8",
    },
    body: JSON.stringify(obj),
};

fetch("http://192.168.2.103:8888/guess.php", options)
```

```php
<?php
header("Access-Control-Allow-Origin: *");
$data = json_decode(file_get_contents('php://input'), true);
$uid = $data['user']['AVUser']['objectId'];
?>
```


参考：https://stackoverflow.com/questions/25727306/request-header-field-access-control-allow-headers-is-not-allowed-by-access-contr


## 解决方法 2：浏览器插件，仅适用于本地调试

## 解决方法 3：JSONP，仅适用于 Get 请求，且需要服务端配合
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

## 解决方法 4：临时关闭浏览器的 CORS 防御策略，仅适用于本地调试
```sh
cd /Applications/Google\ Chrome.app/Contents/
mkdir MyCORS
open -n /Applications/Google\ Chrome.app --args --disable-web-secruity --user-data-dir='/Applications/Google Chrome.app/Contents/MyCORS'
```

