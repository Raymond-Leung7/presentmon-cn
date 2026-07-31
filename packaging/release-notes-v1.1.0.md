# PresentMon CN 1.1.0

基于 PresentMon 上游提交 `e7825637` 制作的首个可直接使用的中文增强发行版。

## 主要内容

- PresentMon 图形界面、指标、统计方式和常用提示的简体中文本地化。
- 改善中文字体与高 DPI 显示效果。
- 修复本地捕获、服务子进程和 ETW 生命周期中的稳定性问题。
- 捕获时自动读取已运行的 HWiNFO Sensors 共享内存，并把传感器列合并到同一份 PresentMon CSV。
- 提供简体中文安装程序和免安装便携 ZIP。
- 使用独立的数据目录、进程通信名称和应用标识，不覆盖 Intel 官方版 PresentMon 的配置。

## 下载选择

- `PresentMon-CN-1.1.0-Setup-x64.exe`：推荐日常使用，按中文向导安装，可创建快捷方式并正常卸载。
- `PresentMon-CN-1.1.0-Portable-x64.zip`：完整解压后，双击根目录的 `PresentMon-CN.exe`。
- `SHA256SUMS.txt`：用于核对下载文件的 SHA-256。

## HWiNFO 联合捕获

HWiNFO 不包含在本发行版内。请先打开 HWiNFO Sensors，并启用 Shared Memory Support（共享内存支持）。之后只需按 PresentMon CN 中设置的捕获快捷键，不需要另外启动 HWiNFO 日志。快捷键不固定为 F1，修改 PresentMon 快捷键后仍会同步生效。

安装版的用户数据保存在“文档\PresentMon-CN”，便携版的数据保存在解压目录的 `app` 文件夹。界面显示“ETL 捕获已禁用”不会影响 CSV 或 HWiNFO 合并数据。

## 签名说明

这是个人构建，目前没有商业代码签名。Windows 首次运行时可能显示“未知发布者”或 SmartScreen 提示，请确认下载来源并核对 `SHA256SUMS.txt`。
