# 仓库维护指南

## 📋 仓库说明

本项目有两个仓库需要维护：

1. **你的个人仓库**（开发仓库）:
   - URL: https://github.com/MarsGao/io.github.MarsGao.speed
   - 用途: 日常开发、测试、Issue 管理
   - Remote 名称: `origin`

2. **Xposed-Modules-Repo 官方仓库**（发布仓库）:
   - URL: https://github.com/Xposed-Modules-Repo/io.github.MarsGao.speed
   - 用途: LSPosed 模块仓库同步，用户通过 LSPosed 管理器安装
   - Remote 名称: `official`

## 🔄 为什么需要两个仓库？

### 原因说明

1. **Xposed-Modules-Repo 要求**:
   - Xposed-Modules-Repo 要求模块必须托管在他们的组织下
   - 仓库名必须等于包名（`io.github.MarsGao.speed`）
   - 这是 LSPosed 模块仓库的标准流程

2. **工作流程**:
   - 你在个人仓库进行开发和测试
   - 稳定版本推送到官方仓库
   - LSPosed 管理器从官方仓库读取模块信息

3. **好处**:
   - 个人仓库：自由开发，不受限制
   - 官方仓库：自动同步到 LSPosed，用户可直接搜索安装

## 🚀 日常维护流程

## 📱 设备侧 LSPosed / Vector 维护记录

### 术语

- LSPosed 中的中文名称：**模块**
- 对应英文名称：**Module**
- 不建议称为“插件 / Plugin”，避免和浏览器插件、Gradle 插件、系统扩展混淆。

### OnePlus Ace 5 基线（2026-06-01）

- 设备：OnePlus `PKG110`
- Android：`16 (API 36)`
- Slot：`_a`
- Kernel：`6.1.118-android14-OP-Wild`
- Root 栈：SukiSU Ultra / Wild Kernel 环境，ADB shell 中 `su` 不可直接访问，但 MMRL 已获得 Root 授权
- LSPosed：`1.9.2 (7024) - Zygisk`
- Xposed API：`100`
- LSPosed 管理器入口：隐藏在 `com.android.shell` pinned shortcut，启动类别为 `org.lsposed.manager.LAUNCH_MANAGER`

### OnePlus 13 基线（2026-06-01）

- 设备：OnePlus `PJZ110`
- Android：`16 (API 36)`
- Slot：`_a`
- Kernel：`6.6.89-android15-OP-WILD`
- Root 栈：KernelSU Next / Wild Kernel 环境，ADB shell 中 `su` 不可直接访问
- LSPosed：`1.11.0 (7209) - Zygisk`
- Xposed API：`100`
- LSPosed 管理器入口：同样隐藏在 `com.android.shell` pinned shortcut，启动类别为 `org.lsposed.manager.LAUNCH_MANAGER`

可用 ADB 启动命令：

```powershell
$Adb = Join-Path $env:LOCALAPPDATA 'Android\Sdk\platform-tools\adb.exe'
& $Adb shell am start -a android.intent.action.MAIN -c org.lsposed.manager.LAUNCH_MANAGER -n com.android.shell/.BugreportWarningActivity
```

### 当前 LSPosed 模块清单（2026-06-01）

#### OnePlus Ace 5

| 模块 | 版本 | 状态 |
|------|------|------|
| 红薯猪手 | `1.2.7` | 未提示更新 |
| 视频调速 VideoSpeed | `1.2.1` | 已验证用于 X/Piko 调速 |
| Freely | `25.1024.0042` | 未提示更新 |
| LuckyTool | `1.3.4` | 已从 `1.3.3` 升级 |
| sing-box | `1.13.12` | 提示需要较新的 Xposed API `101` |
| Telegram Speed Hook | `2.1 AR` | 未提示更新 |
| 隐藏应用列表 / Hide My Applist | `3.8.r499.3a346c0` | 已从 `3.6.1.r462.4524dde` 升级 |

#### OnePlus 13

| 模块 | 版本 | 状态 |
|------|------|------|
| 红薯猪手 | `1.2.7` | 未提示更新 |
| Bili调速 | `1.1.9` | 旧包名保留，未卸载 |
| 视频调速 VideoSpeed | `1.2.1` | 已启用并勾选 `com.twitter.android` |
| LuckyTool | `1.3.4` | 已从 `1.3.3` 升级 |
| OPCameraPro | `Katrina_3.0.42` | `3.1.22` 提示需要 LSPosed v2.0+ / API `101`，暂不升级 |
| sing-box | `1.13.12` | 提示需要较新的 Xposed API `101`，功能正常时暂不迁移框架 |
| Telegram Speed Hook | `2.1 AR` | 未提示更新 |
| 隐藏应用列表 / Hide My Applist | `3.8.r499.3a346c0` | 已从 `3.6.1.r462.4524dde` 升级 |
| NewMiko | `1.8.0` | 未提示更新 |
| WAuxiliary | `1.2.7.r1418.e65079c` | 已从 `1.2.7.r1255.7b778d2` 升级 |

### 已完成的 APK 层更新（2026-06-01）

#### OnePlus Ace 5

- MMRL: `v34242-release` -> `v34296-release`
- Wild KSU Manager: `v0.0.220` -> `v3.1.2`
- LuckyTool: `1.3.3` -> `1.3.4`
- Hide My Applist: `3.6.1.r462.4524dde` -> `3.8.r499.3a346c0`

#### OnePlus 13

- MMRL: `v34265-release` -> `v34296-release`
- LuckyTool: `1.3.3` -> `1.3.4`
- Hide My Applist: `3.6.1.r462.4524dde` -> `3.8.r499.3a346c0`
- WAuxiliary: `1.2.7.r1255.7b778d2` -> `1.2.7.r1418.e65079c`
- VideoSpeed: 安装新版 `io.github.MarsGao.speed 1.2.1`，并在 LSPosed 中启用、勾选 `com.twitter.android`

OnePlus 13 保留项：

- KernelSU Next Manager `v3.1.0` 未升级到 `v3.2.0`，因为新版管理器提高最低内核侧版本要求，不能只升级 APK。
- OPCameraPro `Katrina_3.0.42` 未升级到 `3.1.22`，因为该版本要求 LSPosed v2.0+ / API `101`。
- 旧包名 `com.veo.hook.bili.speed 1.1.9` 未卸载，避免破坏既有作用域；新版 `io.github.MarsGao.speed 1.2.1` 已并存启用。

每次更新后至少确认：

```powershell
& $Adb devices -l
& $Adb shell getprop sys.boot_completed
& $Adb shell ps -A | Select-String "lspd|zygisk|Tricky|integrity"
& $Adb shell dumpsys package <package.name> | Select-String "versionCode=|versionName="
```

### 迁移 Vector / 新 LSPosed 的触发条件

原版 LSPosed `1.9.2 (7024)` 当前还能驱动 VideoSpeed，但它已经不适合作为 Android 16 长期维护基线。满足以下任一条件时，再迁移到活跃分支 Vector / LSPosed：

- LSPosed 模块明确要求 Xposed API `101` 或更高，并且功能已受影响。
- Hide My Applist `3.8+` 因系统服务 native library 加载限制失效。
- Android 系统 OTA 后 `lspd`、`zygisk_lsposed` 不再正常运行。
- VideoSpeed 或其他关键模块在目标应用中无法注入，且日志显示框架层问题。

迁移原则：

- 不在功能正常时盲目替换 LSPosed 核心。
- 只从官方或可信上游下载 Vector / LSPosed zip。
- 一次只替换一个框架模块，重启后验证 `lspd`、`zygisk_lsposed`、LSPosed 管理器、VideoSpeed 和 HMA。
- 若更新后 bootloop，优先从 recovery/fastboot 禁用最近替换的模块，不继续叠加新模块。

### 场景 1: 日常代码更新

当你完成代码修改并提交到个人仓库后：

```bash
# 1. 推送到个人仓库（开发仓库）
git push origin master

# 2. 同步到官方仓库（发布仓库）
git push official master
```

### 场景 2: 发布新版本

当你准备发布新版本时：

```bash
# 1. 更新版本号（在 gradle.properties 中修改 appVersionName）
# 例如: appVersionName=1.2.1

# 2. 提交版本更新
git add gradle.properties
git commit -m "chore: 更新版本号到 1.2.1"
git push origin master

# 3. 创建 tag（LSPosed 格式: VersionCode-VersionName）
# 例如: 1002001-1.2.1
git tag 1002001-1.2.1
git tag v1.2.1  # 可选：同时创建 v 前缀的 tag

# 4. 推送代码和 tags 到两个仓库
git push origin master
git push origin --tags
git push official master
git push official --tags
```

### 场景 3: 快速同步脚本

你可以创建一个 PowerShell 脚本来自动同步：

```powershell
# sync-repos.ps1
Write-Host "正在同步到个人仓库..." -ForegroundColor Green
git push origin master
git push origin --tags

Write-Host "正在同步到官方仓库..." -ForegroundColor Green
git push official master
git push official --tags

Write-Host "同步完成！" -ForegroundColor Green
```

使用方法：
```bash
.\sync-repos.ps1
```

## ⚠️ 重要注意事项

### 1. 仓库描述

官方仓库的描述用于在 LSPosed 中显示模块名称，必须设置：

- 访问: https://github.com/Xposed-Modules-Repo/io.github.MarsGao.speed/settings
- 在 **Description** 字段输入: `视频调速 VideoSpeed - 视频播放速度调节 Xposed 模块`
- 点击 **Save changes**

### 2. Release 要求

- 官方仓库必须至少有一个 Release
- Release tag 格式: `VersionCode-VersionName` (如 `1002000-1.2.0`)
- GitHub Actions 会自动创建 Release（如果配置了 workflow）

### 3. 分支管理

- 当前使用 `master` 分支（官方仓库也使用 master）
- 如果官方仓库要求 `main` 分支，可以创建并推送：
  ```bash
  git checkout -b main
  git push official main
  ```

### 4. 同步时机

- **开发阶段**: 只需推送到个人仓库
- **测试通过后**: 同步到官方仓库
- **发布版本**: 必须同时推送到两个仓库

## 📦 Release 同步（重要）

⚠️ **注意**: 官方仓库的 GitHub Actions 被组织管理员禁用，无法自动构建 Release。

### 自动化同步（推荐）

使用 `sync-release.ps1` 脚本自动同步 Release：

```powershell
# 1. 设置 GitHub Token（首次使用）
$env:GITHUB_TOKEN = "your_token_here"  # 从 https://github.com/settings/tokens 创建

# 2. 同步最新 Release
.\sync-release.ps1

# 或指定特定 Tag
.\sync-release.ps1 1002000-1.2.0
```

详细说明请查看: `sync-release-manual.md`

### 手动同步步骤

1. **下载 APK**: 从个人仓库 Releases 下载 APK
2. **创建 Release**: 在官方仓库创建同名 Release 并上传 APK

## 📝 检查清单

每次更新后，检查以下项目：

- [ ] 代码已推送到个人仓库 (`origin`)
- [ ] 代码已推送到官方仓库 (`official`)
- [ ] Tags 已推送到两个仓库
- [ ] **Release 已同步到官方仓库**（重要！）
- [ ] 官方仓库描述已设置（首次）
- [ ] 等待 10 分钟后，在 LSPosed 中搜索验证

## 🔗 相关链接

- 个人仓库: https://github.com/MarsGao/io.github.MarsGao.speed
- 官方仓库: https://github.com/Xposed-Modules-Repo/io.github.MarsGao.speed
- Xposed-Modules-Repo: https://github.com/Xposed-Modules-Repo
- LSPosed 模块仓库: https://modules.lsposed.org/

## ❓ 常见问题

**Q: 为什么不能直接在官方仓库开发？**  
A: 可以，但个人仓库更灵活，可以自由实验，稳定后再同步到官方仓库。

**Q: 如果忘记同步会怎样？**  
A: 用户无法在 LSPosed 中看到最新版本，但可以通过个人仓库的 Releases 手动下载。

**Q: 两个仓库的代码必须完全一致吗？**  
A: 建议保持一致，特别是 Release 版本。开发中的代码可以先只在个人仓库。

**Q: 如何查看两个仓库的差异？**  
A: 
```bash
# 查看官方仓库的更新
git fetch official
git log master..official/master

# 查看个人仓库的更新
git fetch origin
git log official/master..master
```

