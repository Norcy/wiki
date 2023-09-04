每次提审 AppStore 都要提示缺少合规证明

![](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/620d867b6af24fa397705c040ec6b0ff~tplv-k3u1fbpfcp-zoom-in-crop-mark:1512:0:0:0.awebp#?w=1092&h=666&e=png&b=fefbfa)

其实只要在 Info.plist 添加 App Uses Non-Exempt Encryption 为 NO 即可

即  `<key>ITSAppUsesNonExemptEncryption</key><false/>`

参考：https://m.okjike.com/originalPosts/64d5bddbba30ac5540cfa7ea