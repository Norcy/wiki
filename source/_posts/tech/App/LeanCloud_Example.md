## beforeSave 的时候获取不到对象的属性值
```js
AV.Cloud.beforeSave('GiftUserMap', async function(request) {
  async function reduceDiamond() {
    let gift = request.object.get('gift');
    let giftName = gift.get('name')
    // giftName is undefined
  }
}
```

## 创建对象
```js
// 方法 1：直接使用 AV.Object 的构造器
const reward = new AV.Object('Reward');
/* 方法 2：创建 AV.Object 子类，然后创建实例
const Reward = AV.Object.extend('Reward');
const reward = new Reward();
*/
reward.set('name', 'Q 币 5 个');
reward.save().then((reward) => {
  console.log('Save Success')
})
```

## 获取对象
根据键值对，返回数组

```js
new AV.Query('Reward')
  .equalTo('objectId', options.id)
  .find()
  .then(result => {
    let simpleResult = jsonify(result)[0];
    console.log(simpleResult);
    this.setData({reward: simpleResult});
  })
  .catch(console.error);
```

根据 ID，返回单个值

```js
const query = new AV.Query('Reward');
query.get(options.id).then((result) => {
  let simpleResult = jsonify(result);
  console.log(simpleResult)
  this.setData({reward: simpleResult});
});
```

## 一些常用的查询

```js
// 等于
query.equalTo('firstName', 'Jack');

// 不等于
query.notEqualTo('firstName', 'Jack');

// 查找不包含 'doubanId' 字段的对象，doubanID 为 undefined
query.doesNotExist('doubanId);

// 查找包含 'doubanId' 字段的对象，doubanID 不为 undefined
query.exists('doubanId);

// 限制 age < 18
query.lessThan('age', 18);

// 限制 age <= 18
query.lessThanOrEqualTo('age', 18);

// 限制 age > 18
query.greaterThan('age', 18);

// 限制 age >= 18
query.greaterThanOrEqualTo('age', 18);

// 最多获取 10 条结果
query.limit(10);

// 只需要第一个结果
query.first()

// 跳过前 20 条结果
query.skip(20);

// 组合查询（连续写就行了）实现翻页效果
const query = new AV.Query('Todo');
query.limit(10);
query.skip(20);

// 【逻辑与】和【逻辑或】组合查询
const priorityQuery = new AV.Query('Todo');
priorityQuery.greaterThanOrEqualTo('priority', 3);
const isCompleteQuery = new AV.Query('Todo');
isCompleteQuery.equalTo('isComplete', true);
const query = AV.Query.or(priorityQuery, isCompleteQuery);

// 双表查询
const studentQuery = new AV.Query('Student');
const countryQuery = new AV.Query('Country');
// 获取所有的英语国家
countryQuery.equalTo('language', 'English');
// 把 Student 的 nationality 和 Country 的 name 关联起来
studentQuery.matchesKeyInQuery('nationality', 'name', countryQuery);
studentQuery.find().then((students) => {
  // students 包含 John Doe 和 Tom Sawyer
});

// 指定需要返回的属性
const query = new AV.Query('Todo');
query.select(['title', 'content']);
query.first().then((todo) => {
  const title = todo.get('title'); // √
  const content = todo.get('content'); // √
  const notes = todo.get('notes'); // undefined
});

// 统计数量
const query = new AV.Query('Todo');
query.equalTo('isComplete', true);
query.count().then((count) => {
  console.log(`${count} 个 todo 已完成。`);
});

// 指针和 include 的配合使用，减少网络请求
const query = new AV.Query('Comment');
// 从所有评论中获取 10 条
query.limit(10);
// 查询的同时会把博客文章请求回来
query.include('post');
query.find().then((comments) => {
  // comments 包含最新发布的 10 条评论，包含各自对应的博客文章
  comments.forEach((comment) => {
    // 该操作无需网络连接
    const post = comment.get('post');
  });
});
```



## 关联查询
### 创建关联
方法 1：设置 parent 字段，是指针吗？

```js
// 创建 post
const post = new AV.Object('Post');
post.set('title', '饿了……');
post.set('content', '中午去哪吃呢？');

// 创建 comment
const comment = new AV.Object('Comment');
comment.set('content', '当然是肯德基啦！');

// 将 post 设为 comment 的一个属性值
comment.set('parent', post);

// 保存 comment 会同时保存 post
comment.save();
```

方法 2：使用 createWithoutData 获取指针（建议）

```js
// 之前已经创建好的 Post，注意这里并没有发送网络请求，只是创建了一个指针
const post = AV.Object.createWithoutData('Post', '57328ca079bc44005c2472d0');

// 创建 comment
const comment = new AV.Object('Comment');
comment.set('content', '当然是肯德基啦！');
// 指向指针
comment.set('post', post);
```

### 查询关联（基于方法 2）
```js
// 查询该 Post 的所有 Comments
const query = new AV.Query('Comment');
query.equalTo('post', post);
query.find().then((comments) => {
  // comments 包含与 post 相关联的评论
});
```

```js
// 查询所有【包含图片的文章】的所有评论
const innerQuery = new AV.Query('Post');
// 得到所有包含图片的文章
innerQuery.exists('image');

const query = new AV.Query('Comment');
// 从所有评论里面挑选跟【查询到的文章】相关的评论
query.matchesQuery('post', innerQuery);
```

### 多对多（重要！）
```js
var studentTom = new AV.Object('Student');
studentTom.set('name', 'Tom');

// 使用已有的对象
var courseLinearAlgebra = AV.Object.createWithoutData('Course', '线性代数');

var studentCourseMapTom = new AV.Object('StudentCourseMap');
studentCourseMapTom.set('student', studentTom);
studentCourseMapTom.set('course', courseLinearAlgebra);

// 注意这个操作会同时更新两个表，Student 和 StudentCourseMap
studentCourseMapTom.save();
```


## 同步对象
```js
const todo = AV.Object.createWithoutData('Todo', '582570f38ac247004f39c24b');
todo.fetch().then((todo) => {
  // todo 已刷新
});

// 指定需要被刷新的属性
todo.fetch({
  keys: 'priority, location'
}).then((todo) => {
  // 只有 priority 和 location 会被获取和刷新
});
```

## 更新对象
```js
const todo = AV.Object.createWithoutData('Todo', '582570f38ac247004f39c24b');
// 这样只会更新指定字段
todo.set('content', '这周周会改到周三下午三点。');
todo.save();
```

## 有条件更新对象
```js
const account = AV.Object.createWithoutData('Account', '5745557f71cfe40068c6abe0');
// 对 balance 原子减少 100
const amount = -100;
account.increment('balance', amount);
account.save(null, {
  // 设置条件：Account.balance >= 100 才执行
  query: new AV.Query('Account').greaterThanOrEqualTo('balance', -amount),
  // 操作结束后，返回最新数据。
  // 如果是新对象，则所有属性都会被返回，
  // 否则只有更新的属性会被返回。
  fetchWhenSave: true
}).then((account) => {
  console.log(`当前余额为：${account.get('balance')}`);
}, (error) => {
  if (error.code === 305) {
    console.error('余额不足，操作失败！');
  }
});
```

## 原子计数器
可以通过原子操作来增加或减少一个属性内保存的数字，用于多个客户端同时修改一个数字

```js
post.increment('likes', 1);
```

## 原子数组操作
更新数组也是原子操作。使用以下方法可以方便地维护数组类型的数据：

```js
const todo = new AV.Object('Todo');
// 将指定对象附加到数组末尾
todo.add('arrayKey', value)
// 如果数组中不包含指定对象，则将该对象加入数组。对象的插入位置是随机的
todo.addUnique('arrayKey', value)
// 从数组字段中删除指定对象的所有实例
todo.remove('arrayKey', value)
```

## 如果你想导出数据/批量新增数据供 App 读取/备份
[RESTAPI](https://leancloud.cn/docs/rest_api.html)



## 权限管理
1. 设置好各个 Class 的增删改查的权限
2. 使用 masterKey 来操作后台数据

```js
AV.init({
    appId: "abccc",
    appKey: "abc",
    serverURL: "https://xxx.com",
    masterKey: "xxx"
});
AV.debug.enable();
AV._config.useMasterKey = true;
```
