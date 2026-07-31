# PresentMon 中文本地化版

这是 [PresentMon](README.md) 的简体中文本地化与本地增强版本，适合在中文 Windows 环境中编译和使用。项目保留上游的数据格式兼容性，并增加了 HWiNFO 传感器联合捕获能力。

## 直接下载使用

普通用户不需要安装 Visual Studio，也不需要自己编译。请打开 [Releases](https://github.com/Raymond-Leung7/presentmon-cn/releases) 并选择以下一种版本：

- 安装版 `PresentMon-CN-*-Setup-x64.exe`：推荐日常使用。双击后按中文向导安装，可创建开始菜单和桌面快捷方式，也可从 Windows“已安装的应用”中卸载。
- 便携版 `PresentMon-CN-*-Portable-x64.zip`：适合放在移动硬盘或单独文件夹。完整解压后，双击根目录的 `PresentMon-CN.exe`，不要只取出一个 EXE。

安装版的捕获、布局和偏好设置默认保存在“文档\PresentMon-CN”，日志与浏览器缓存保存在 `%LOCALAPPDATA%\Raymond-Leung7\PresentMon-CN`。卸载时会清理日志和缓存，但保留“文档”中的用户数据。便携版的捕获、配置和日志保存在解压目录的 `app` 文件夹中。两种发行版都使用独立的进程通信名称和数据目录，不会覆盖 Intel 官方版 PresentMon 的配置。

当前个人发行版没有商业代码签名，Windows 首次运行时可能显示“未知发布者”或 SmartScreen 提示。请只从本仓库 Release 页面下载，并使用同一页面的 `SHA256SUMS.txt` 核对文件。

## 汉化范围

当前主要覆盖 Intel PresentMon 图形界面，包括：

- 首页、状态提示和常用操作；
- 捕获、叠加层、数据、日志及其他设置；
- 布局、图表、读数和快捷键编辑界面；
- 95 项指标、16 种统计方式及常用枚举读数的中文显示；
- 常见确认框、错误信息和通知。

少量由 Windows、CEF 或第三方组件提供的文字仍可能显示为英文。命令行版及开发者文档以英文为主。

## 本地增强功能

- 改善高 DPI 显示和中文字体清晰度；
- 修复本地 Debug 构建在缺少图形调试层时的启动问题；
- 加强捕获开始、停止和事件处理过程的稳定性；
- 捕获时直接读取已运行的 HWiNFO 共享内存，将传感器数据追加到同一份 PresentMon CSV；
- HWiNFO 联合捕获跟随 PresentMon 的捕获状态，因此修改 PresentMon 捕获快捷键后仍然有效。

使用联合捕获前，请保持 HWiNFO Sensors 窗口运行，并在 HWiNFO 设置中启用共享内存支持。无需单独启动 HWiNFO 日志记录。

捕获快捷键不固定为 F1。你在 PresentMon 中文版中改成什么键，联合捕获就跟随什么键。界面显示“ETL 捕获已禁用”只表示不额外保存原始 ETL 文件，不影响 CSV 或 HWiNFO 传感器列。

## 为什么部分内容保留英文

为保证与上游版本、旧配置和外部工具兼容，下列内容不会强制翻译：

- `PresentMon`、`API`、`CEF`、`ETW`、`ETL`、`GPU`、`CPU` 等名称；
- `fps`、`Hz`、`ms`、`W` 等单位和常用统计缩写；
- 内部指标名称、CSV 列名、JSON 字段和配置签名；
- 命令行参数、文件名、扩展名、服务名、路由及枚举值；
- 显卡、进程和硬件返回的原始名称。

这些英文项属于接口或数据格式的一部分，修改后可能导致旧配置无法读取、脚本失效或数据字段不一致。

## 首次构建

完整依赖要求请参阅 [BUILDING.md](BUILDING.md)。需要 Visual Studio 2022、vcpkg、CMake、Node.js/NPM，以及 WiX Toolset v3 和对应的 Visual Studio 扩展。以下命令建议在 **Developer PowerShell for VS 2022** 中运行。

请在 Visual Studio Installer 中安装 C++ vcpkg 组件，并先运行一次 `vcpkg integrate install`。如果构建时提示找不到 `CLI/CLI.hpp`、Boost 或 cereal，通常就是 vcpkg 尚未集成或依赖尚未还原。

首次拉取 CEF、测试数据并构建前端：

```powershell
Set-Location <仓库目录>
.\bootstrap.ps1
```

如果只需要首次安装前端依赖并构建中文界面：

```powershell
Set-Location .\IntelPresentMon\AppCef\ipm-ui-vue
npm ci
npm run build
Set-Location ..\..\..
```

随后在仓库根目录构建 Debug x64 原生程序：

```powershell
msbuild PresentMon.sln /m /p:Platform=x64 /p:Configuration=Debug
```

Debug 构建不需要 Release 版本使用的测试证书。

## 构建发行包

准备好完整的 Release 依赖后，可在仓库根目录运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\packaging\build-release.ps1 -Version 1.1.3
```

脚本会构建无控制台窗口的启动器、便携 ZIP、当前用户安装程序和 SHA-256 校验文件，输出到 `dist` 文件夹。安装程序需要 Inno Setup 6；只需要便携包时可添加 `-SkipInstaller`。

## 运行

成功构建后，主程序和本地中间件分别位于 `build\Debug\PresentMon.exe` 与 `build\Debug\PresentMonAPI2.dll`。

在本工作区中，最简单的方式是直接双击仓库根目录的 `启动汉化版.cmd`。请不要直接双击 `build\Debug\PresentMon.exe`：裸程序默认会连接电脑中已安装的 PresentMon 服务，官方版本与当前源码版本不一致时会直接退出。

本地调试最简单的方式是让 PresentMon 把服务作为子进程启动：

```powershell
Set-Location .\build\Debug
.\PresentMon.exe --svc-as-child --files-working --log-level verbose --middleware-dll-path .\PresentMonAPI2.dll --log-middleware-copy
```

这种方式无需手动安装 `PresentMonService`。如需单独安装和管理服务，请按 [BUILDING.md](BUILDING.md#presentmon-service) 中的说明，以管理员身份运行 `sc.exe`。

## 权限和常见问题

- 安装版和便携版启动器会自动检查捕获权限。不在“性能日志用户”本地组时，可请求一次管理员授权，仅把当前 Windows 账户加入该本地组；修复完成后需要注销并重新登录。主程序本身始终以普通权限启动。
- 卸载 PresentMon CN 不会自动把账户移出“性能日志用户”组，避免撤销由其他软件或管理员配置的同一项 Windows 权限。
- 请不要对 `PresentMon-CN.exe` 使用“以管理员身份运行”。如果目标游戏以管理员身份运行，建议先取消游戏的管理员兼容性设置，再进行捕获。
- 若 1.1.2 出现黑屏且关闭后仍留在后台，请先重启 Windows 清除旧进程，再覆盖安装 1.1.3 或更高版本。
- 手动创建、启动或删除 Windows 服务需要管理员权限；使用 `--svc-as-child` 通常更适合本地开发。
- 发行版使用独立的子服务和通信名称，可与已安装的官方版共存。直接运行源码目录中的裸 `PresentMon.exe` 时，才需要注意它可能连接到版本不匹配的官方服务。
- 界面资源缺失或启动后显示空白时，重新运行 `.\bootstrap.ps1`，或进入前端目录执行 `npm ci` 和 `npm run build`。
- Debug 程序可能触发 Windows Defender 或 SmartScreen 提示；只放行由自己从可信源码构建的文件。
- 本仓库的发行脚本会生成无需测试证书的个人 Release 包；公开分发时仍建议使用正式代码签名证书。

## 同步上游更新

本仓库使用 `origin` 指向中文增强版仓库，使用 `upstream` 指向 PresentMon 官方仓库。同步前应提交当前改动，并确认工作区没有未保存的文件：

```powershell
git fetch upstream
git switch codex/presentmon-cn
git merge upstream/main
```

合并后重点检查 `IntelPresentMon\AppCef\ipm-ui-vue` 下的汉化冲突、HWiNFO CSV 列结构和捕获生命周期，再重新运行前端构建、Debug x64 构建、单元测试与实际捕获验证。

更多原始说明请参阅上游的 [README.md](README.md)、[BUILDING.md](BUILDING.md) 和 [捕获应用文档](README-CaptureApplication.md)。
