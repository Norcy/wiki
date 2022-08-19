本文记录一些 LeanCloud 方面的安全实践

## MasterKey
MasterKey 是级别最高的 Key，会跳过所有 API 的权限检查，绝对不能泄漏

1. 不能把 MasterKey 放到公开的代码里
2. 如果 MasterKey 有曾经泄漏的可能性，则需要到控制台重置

## AppKey
AppKey 是公开的访问密钥，适用于在公开的客户端中使用。使用 AppKey 进行的访问受到 ACL 的限制

1. AppKey 是内置在客户端代码中使用的
2. 普通的 LeanCloud 请求会对 AppKey 进行加密传输而不是明文，因此抓包是无法获取 AppKey
3. AppKey 依然有泄漏的可能性，比如通过反编译你的 App，或抓取你的 JSBundle 来获取，需要其他手段（比如 JS 混淆）来保障安全


## 表权限
表的权限如下

1. `add_fields`：给 Class 增加新的字段，也就是说，保存对象时，如果对应列不存在，是否允许自动创建新的字段。对所有用户关闭。需要创建表的列在控制台创建即可，而不是客户端
2. create：在 Class 表中插入一个新对象。按需设置。比如需要登录用户才能创建一条 Record，此时 Record 的 create 权限可以改为登录用户
3. delete：删除既有对象。按需设置
4. update：修改既有对象。按需设置
5. find：通过指定条件查询对象。关闭此权限时，无法查询对象，只能通过 objectId 获取对象（需开启 get 权限），一定程度上可以防范别有用心的人批量抓取该 Class 下的所有对象
6. get：通过 objectId 获取单个对象。按需设置

## 列权限
1. 按需设置即可，一般建议把 ACL/createdAt/updatedAt 都改为只读和客户端不可见

## 对象 ACL 权限
对象，即表格中的每一行。每一个对象都有一个 ACL 字段，这可以提供最精细化的权限控制

1. 一般由用户产生的数据（比如书影记录、User 表等），建议设置**默认 ACL 权限**，让 read 和 write 的权限都为 owner。这样可以防止所有记录被恶意批量删除

## 实践：User
对于表权限：建议对所有人关闭 `add_fields`、delete、find、get；对所有人开启 create，否则未登录用户无法创建；只对已登录用户开放 update，但是这种无法规避已登录的有心人士篡改其他登录用户的数据，所以还是得靠对象 ACL 权限

对于对象 ACL 权限：应该为自己，但是不能简单设置为 Owner，因为创建 User 的时候，User 的 Owner 并不确定；所以得在 User 对象创建之后再更新其 ACL

云函数代码如下

```js
const AV = require('leanengine');

AV.Cloud.afterSave('_User', function (request) {
  var user = request.object;

  var acl = new AV.ACL();
  acl.setPublicReadAccess(false);
  acl.setPublicWriteAccess(false);
  acl.setReadAccess(user.id, true);
  acl.setWriteAccess(user.id, true);

  user.setACL(acl);

  return user.save().then(function (res) {
    console.log(res, 'User Update ACL Success!');
  });
});

```



## 参考
更详细的可以参考 [LeanCloud 数据和安全](https://leancloud.cn/docs/data_security.html)