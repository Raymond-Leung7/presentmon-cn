PresentMon CN 使用说明
=====================

一、便携版

1. 完整解压 ZIP，不能只把单个 EXE 拖出来。
2. 双击 PresentMon-CN.exe 启动。
3. 捕获文件保存在 app\Captures 文件夹。

二、安装版

1. 双击 PresentMon-CN-<版本>-Setup-x64.exe，按向导安装。
2. 从开始菜单或桌面快捷方式启动 PresentMon CN。
3. 捕获文件默认保存在“文档\PresentMon-CN\Captures”。
4. 可在 Windows“已安装的应用”中正常卸载。
5. 卸载会清理程序、缓存和日志，但保留“文档\PresentMon-CN”中的捕获、布局与偏好设置。

首次启动时，如果当前账户还没有捕获权限，软件会询问是否修复。选择“是”并完成一次管理员授权后，请注销 Windows 并重新登录，再普通启动 PresentMon CN。管理员授权只用于把当前账户加入“性能日志用户”组，不会以管理员身份运行主程序。
卸载 PresentMon CN 不会自动把账户移出“性能日志用户”组，避免撤销由其他软件或管理员配置的同一项 Windows 权限。

三、同步捕获 HWiNFO 数据

1. 启动 HWiNFO Sensors，并在设置中启用 Shared Memory Support（共享内存支持）。
2. 保持 HWiNFO Sensors 运行，不需要手动开始 HWiNFO 日志。
3. 在 PresentMon CN 中按你设置的捕获快捷键。
4. 同一个快捷键会控制 PresentMon 捕获；程序会在捕获期间自动读取 HWiNFO 共享内存，并把传感器列合并进同一份 CSV。

快捷键不是固定 F1。你在 PresentMon CN 中改成什么键，就使用什么键。

四、常见问题

- “ETL 捕获已禁用”只表示不另外保存原始 ETL 跟踪，不影响 CSV 和 HWiNFO 合并数据。
- 请不要对 PresentMon-CN.exe 使用“以管理员身份运行”。如果目标游戏以管理员身份运行，建议先取消游戏的管理员兼容性设置。
- 如果 1.1.2 曾出现黑屏且关闭后仍留在后台，请先重启 Windows，再安装新版。
- 本项目不会安装、修改或卸载 HWiNFO。
- 当前个人构建没有商业代码签名。Windows 首次运行安装包时可能显示“未知发布者”；请先核对 Release 页面提供的 SHA-256。

项目主页：
https://github.com/Raymond-Leung7/presentmon-cn

上游项目：
https://github.com/GameTechDev/PresentMon
