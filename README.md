# MMD-Desktop-mascot-
MMD桌面精灵Desktop mascot，支持pmx、pmd、vmd，基于SabaMMD解析库。

桌面精灵功能包括：
1. 加载pmx，pmd，vmd
2. 异型窗口自由拖动，模型自由移动，模型360°全方位旋转（球面轨迹摄像机，有小bug）
3. 自动切换vmd，循环加载vmd（只要你不关闭，她会一直不停的为你跳一支舞）
4. 自定义窗口位置和宽高
5. 自定义鼠标左键，右键，中键以及缩放灵敏度设置。
6. 支持抗锯齿，自定义抗锯齿层数
7. 支持向模型窗口拖入pmx，pmd，vmd（拖入的vmd会添加进循环vmd列表），config.ini（在不重新启动程序的情况下初始化所有配置）。
8. 考虑到窗口一直置顶可能会妨碍工作，因此加入了用户选择是否置顶设置。
9. 优化了vmd切换的等待时间间隔
10. 优化了影子透明度和方向
11.优化CPU占有率，可自行设置单核或者多核运行，以及指定单核核心
12.增加摄像机目标追踪
13. 舞蹈顺序更换为vmd随机切换

缺点：
1. 暂时不支持界面交互
2. 部分计算机可能缺少c运行库。我已经在一个不编程的朋友电脑上测试过，完美运行，所以大部分应该都没问题（win10）。

软件功能演示：https://www.bilibili.com/video/BV15T4y1G7Gj

my_glfw.cpp使用说明：
1. 搭建好saba的运行环境，并成功运行saba_viewer.cpp。
2. 将my_glfw.cpp放在项目simple_mmd_viewer_glfw下，Release运行。

不想自己搭建的，可以下release版本--MMD桌面精灵release1.0.rar--，我已经编译好的，可直接运行。

Saba：https://github.com/benikabocha/saba
