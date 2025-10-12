## 背景

当某些第三方库只支持特定平台时（如 `react-native-notifications` 只支持 iOS），我们需要在不支持的平台上排除这些依赖，避免应用崩溃。

React Native 应用分为两层：
- **JavaScript 层**：可以正常 import，不会崩溃
- **原生层**：需要原生代码支持，调用时可能崩溃

以下是四种排除方法的对比和使用场景。

## 方法一：react-native.config.js 配置（推荐）

### 基本用法

在项目根目录创建 `react-native.config.js`：

```javascript
module.exports = {
  dependencies: {
    'react-native-notifications': {
      platforms: {
        android: null, // 排除 Android 平台的原生链接
      },
    },
  },
};
```

### 特点
- **优点**：配置简单，JavaScript 层完全可用
- **缺点**：调用原生功能时仍会报错
- **适用场景**：大部分情况的首选方案

### 注意事项
```typescript
// ✅ 可以正常使用
import { Notification } from 'react-native-notifications';
const { Notifications } = require('react-native-notifications');

// ❌ 调用原生功能会报错
Notifications.registerRemoteNotifications(); // 需要配合平台检查
```

## 方法二：平台特定文件

### 文件命名规则

React Native 支持平台特定的文件扩展名：

```
MyComponent.js          // 通用文件
MyComponent.ios.js      // iOS 专用
MyComponent.android.js  // Android 专用
MyComponent.native.js   // 原生平台通用（排除 Web）
```

### 示例

**Pusher.ios.ts**：
```typescript
import { Notifications } from 'react-native-notifications';

export const Pusher = {
  init: () => Notifications.registerRemoteNotifications(),
  hasSupport: () => true
};
```

**Pusher.android.ts**：
```typescript
export const Pusher = {
  init: () => console.log('Not supported'),
  hasSupport: () => false
};
```

**使用**：
```typescript
import { Pusher } from './Pusher'; // 自动选择平台文件
Pusher.init();
```

### 特点
- **优点**：完全隔离，类型安全
- **缺点**：需要维护多个文件
- **适用场景**：平台差异较大的复杂逻辑

## 方法三：运行时平台检查（最常用）

### 示例

```typescript
import { Platform } from 'react-native';

const getNotifications = () => {
  if (Platform.OS === 'ios') {
      return require('react-native-notifications').Notifications;
  }
  return null;
};

export const PushService = {
  register: () => {
    const notifications = getNotifications();
    if (notifications) {
      notifications.registerRemoteNotifications();
    }
  }
};
```

### 特点
- **优点**：灵活，可以动态判断
- **缺点**：代码中需要大量平台检查
- **适用场景**：需要动态行为或少量平台差异


## 方法四：Metro 配置排除

### 示例

```javascript
// metro.config.js
const { Platform } = require('react-native');

module.exports = {
  resolver: {
    blacklistRE: Platform.OS === 'android' 
      ? /react-native-notifications/ 
      : undefined,
  },
};
```

### 特点
- **优点**：彻底排除，包体积更小
- **缺点**：JavaScript 层也无法使用，过于激进
- **适用场景**：确定完全不需要该模块的极端情况

### 注意事项
```typescript
// ❌ 连 import 都会失败
import { Notifications } from 'react-native-notifications'; // 模块不存在

// ❌ require 也会失败
const notifications = require('react-native-notifications'); // 找不到模块
```

## 方法选择建议

| 方法 | 复杂度 | 安全性 | 适用场景 |
|------|--------|--------|----------|
| react-native.config.js | ⭐ | ⭐⭐⭐ | 首选方案 |
| 平台特定文件 | ⭐⭐ | ⭐⭐⭐⭐ | 复杂逻辑 |
| 运行时检查 | ⭐⭐⭐ | ⭐⭐ | 动态需求 |
| Metro 排除 | ⭐ | ⭐⭐⭐⭐ | 极端情况 |

**推荐组合**：`react-native.config.js` + 运行时平台检查，既简单又安全。
