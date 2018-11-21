主要是 [XcodeProj](https://github.com/CocoaPods/Xcodeproj) 这个工具的应用

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
