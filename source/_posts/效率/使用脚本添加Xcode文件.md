主要是 [XcodeProj](https://github.com/CocoaPods/Xcodeproj) 这个工具的应用

## Xcode 中 Group 和 FileRef 的区别
### Group
Group 这个概念和我们平时经常说的 folder 文件夹有很大的差别, 文件夹在我们的日常使用时是一直所接触到的, 而对于 Group, 如果你不使用 Xcode 来编程(不是很清楚别的 IDE 是否有这个功能)的话, 这个概念距离你太远了.

Group 其实是 Xcode 中用来组织文件的一种方式, 它对文件系统没有任何影响, 无论你创建或者删除一个 Group, 都不会导致 folder 的增加或者移除. 当然如果在你删除时选择 Move to Trash 就是另外一说了.

在 Group 中的文件的关系, 不会与 folder 中的有什么冲突, 它只是 Xcode 为你提供的一种分离关注的方式而已. 但是, 我一般会在开发过程中将不同的模块分到不同的 Group 和 folder 中便于整理.

Group 之间的关系, 也是在 project.pbxproj 中定义的, 这个文件中包含了 Xcode 工程中所有 File 和 Group 的关系, 如果你大致浏览过这个文件的话, 你就会对我所说的有所了解.

Group 在我们的工程中就是黄色的文件夹, 而 Folder 是蓝色的文件夹

一般在 Xcode 工程中, 我们比较少用 Folder

Folder 会被打入到 app 的 bundle 中，比如 H5 的离线包文件夹就可以使用

### FileRef
FileRef 其实就是 File Reference 的缩写, 当你从 Xcode 中删除一个文件的时候, 它会弹出提示框.

而其中的 Remove Reference 选项并不会将这个文件移除到垃圾桶, 而只是会将这个文件的引用从 Xcode 的工程文件中删除.

如果你曾经看过 Build Phases 中的内容, 你会发现

如果删除的是 .h 文件, 它会从 Build Phases 中的 Headers 部分删除
如果删除的是 .m 文件, 它会从 Build Phases 中的 Compile Source 部分删除
但是文件还是会在原来的地方, 因为 Xcode 中所加入到工程的只是文件的一个引用 — File Ref.

## 代码实现

```ruby
require 'xcodeproj'
project_path = './VideoNativeiPhone.xcodeproj'
project = Xcodeproj::Project.open(project_path)

# 测试的目录结构
=begin
./
├── *.xcodeproj
├── This
	└── is
	    └── Test
	        ├── Test.h
	        └── Test.m
=end

# 递归从 Build Phases 删除引用
def removeBuildPhaseFilesRecursively(aTarget, aGroup)
  aGroup.files.each do |file|
      if file.real_path.to_s.end_with?(".m", ".mm", ".cpp") then
          aTarget.source_build_phase.remove_file_reference(file)
      elsif file.real_path.to_s.end_with?(".plist") then
          aTarget.resources_build_phase.remove_file_reference(file)
      end
  end
  
  aGroup.groups.each do |group|
      removeBuildPhaseFilesRecursively(aTarget, group)
  end
end


# 枚举所有 target
project.targets.each do |target|
  puts target.name
end

# 获取第一个 Target，一般与工程名字同名
target = project.targets.first

# 根据路径名寻找 group。如果 should_create = true，则如果当前的 group 不存在, 它还会递归地创建
group = project.main_group.find_subpath(File.join('This', 'is', 'Test'), true)

# 建议设置为 SOURCE_ROOT，为了加入到 Build Phases 的时候, 从工程文件的根目录下开始寻找你所添加的文件
# <absolute> for absolute paths
# <group> for paths relative to the group
# SOURCE_ROOT for paths relative to the project
# DEVELOPER_DIR for paths relative to the developer directory.
# BUILT_PRODUCTS_DIR for paths relative to the build products directory.
# SDKROOT for paths relative to the SDK directory.
group.set_source_tree('SOURCE_ROOT')

# 如果多次添加会导致重复，因此需要将引用和 build phases 的值删除
if !group.empty? then
	removeBuildPhaseFilesRecursively(target, group)
    group.clear()
end

# 为文件创建一个 FileRef 添加到 group 中
file_ref_h = group.new_reference("./This/is/Test/Test.h")

file_ref_m = group.new_reference("./This/is/Test/Test.m")

# .h文件只需引用一下，.m 引用后还需加入 Build Phases
target.add_file_references([file_ref_m])

# 保存
project.save
```

## 参考文章
+ [使用代码为 Xcode 工程添加文件](https://draveness.me/bei-xcodeproj-keng-de-zhe-ji-tian)（入门篇）
+ [懒人福利：用脚本来修改Xcode工程](http://blog.wtlucky.com/blog/2016/10/10/use-script-to-modify-xcode-project/)（实战篇）
+ [XcodeProj 源码](https://github.com/CocoaPods/Xcodeproj)
