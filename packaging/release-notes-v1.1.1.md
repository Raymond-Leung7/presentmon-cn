# PresentMon CN 1.1.1

基于 PresentMon 上游提交 `e7825637` 制作的高 DPI 界面修复版。

## 本次修复

- 修复 Windows 使用 125%、150%、200% 等显示缩放时，首次打开软件后页面整体偏右、右侧和底部被裁切的问题。
- CEF 浏览器创建完成后会立即与主窗口客户区重新对齐，不再需要手动拖动或缩放窗口才能恢复。
- 保留 Per-Monitor V2 高 DPI 支持，中文字体继续清晰显示，不会退回模糊的 DPI 兼容模式。
- 加强主页面的宽度约束，降低较窄窗口中出现横向溢出的风险。

## 下载选择

- `PresentMon-CN-1.1.1-Setup-x64.exe`：推荐日常使用，可直接覆盖安装 1.1.0。
- `PresentMon-CN-1.1.1-Portable-x64.zip`：完整解压后，双击根目录的 `PresentMon-CN.exe`。
- `SHA256SUMS.txt`：用于核对下载文件的 SHA-256。

## 使用说明

HWiNFO 联合捕获方式没有变化：先打开 HWiNFO Sensors 并启用 Shared Memory Support，之后只需使用 PresentMon CN 中设置的捕获快捷键。安装升级不会删除“文档\PresentMon-CN”中的个人配置和捕获数据。

本项目为个人构建，目前没有商业代码签名。Windows 可能显示“未知发布者”或 SmartScreen 提示，请确认下载来源并核对校验文件。
