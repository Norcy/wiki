---
title: Hexo 特性
---


## 一些坑
+ 如果使用了分割线，则生成 html 失败（已经第三次踩坑），如果要用分割线，得使用 `____` 而不是 `---`

## Travis CI 自动构建

## [NexT 主题添加点击的爱心效果](https://asdfv1929.github.io/2018/01/26/click-love/)

## [NexT 主题内接入网页在线联系功能](https://asdfv1929.github.io/2018/01/21/daovoice/)

## [NexT 主题中添加网页标题崩溃欺骗搞怪特效](https://asdfv1929.github.io/2018/01/25/crash-cheat/)

## NexT 主题添加圆角效果
以 Pisces 模式为例，其他模式同理

+ `themes/next/source/css/_schemes/Pisces/_sidebar.styl` 添加 `border-radius` 属性

    ```css
    .sidebar {
      display: none;
      right: auto;
      bottom: auto;
      -webkit-transform: none;
      border-radius:10px;
    }
    
    .sidebar-inner {
      box-sizing: border-box;
      width: 240px;
      color: $text-color;
      background: white;
      border-radius:10px;
      &.affix { top: 0px; position: fixed; }
    }
    ```

+ `themes/next/source/css/_schemes/Pisces/_layout.styl`添加 `border-radius` 属性

    ```css
    .sidebar-position-right {
      .header-inner { right: 0; border-radius:10px;}
      .content-wrap { float: left; }
      .sidebar { float: right; }
    
      .footer-inner:before { float: right; }
    }
    ```

## NexT 主题添加动态背景
`themes/next/layout/_layout.swig` 添加以下代码

```js
{% if theme.canvas_nest %}
<!-- 可选配置：color="0,255,255" opacity="0.7" count=99 zIndex="0"-->
<script type="text/javascript" color="51,163,220" opacity="0.8" src="//cdn.bootcss.com/canvas-nest.js/1.0.0/canvas-nest.min.js"></script>
{% endif %}
```


## 不蒜子统计

