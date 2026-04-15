---
id: 模块-app/19-Linux深色主题检测
status: in_progress
owners: [@pcfan]
code: [src/app/MainWindow.cpp]
tests: []
depends: [specs/横切关注点/30-主题系统.md, specs/模块-app/12-主题插件系统.md]
last_reviewed: 2026-04-15
---

# Linux 深色主题自动检测

## 1. 目的

Windows 已能通过读取 `HKCU\...\Personalize\AppsUseLightTheme` 注册表项自动跟随系统深色 / 浅色模式。Linux 之前只走 `QPalette::Window` 亮度启发式，在 GTK ↔ Qt 主题桥接的发行版上不准。本 Spec 定义 Linux 下的三级探测策略。

V1 只支持**启动时 / 手动选"跟随系统"时**的快照检测。**不做** DBus 实时监听（用户切系统主题后需要手动重新选"视图 → 主题 → 跟随系统"或重启应用，V2 再做）。

## 2. 输入 / 输出

- **输入**：Linux 桌面环境设置
  - GNOME 42+：`gsettings get org.gnome.desktop.interface color-scheme`
  - KDE Plasma：`~/.config/kdeglobals` 的 `[General] ColorScheme`
  - 其他 DE：Qt 的 `QApplication::palette().color(Window)`
- **输出**：`bool isSystemDarkMode() const`，true = 深色

## 3. 行为契约（不变量）

### [INV-1] 三级探测顺序

必须按以下顺序执行，首个**明确**的结果立即 return：

1. **GNOME gsettings**：QProcess 调 `gsettings get org.gnome.desktop.interface color-scheme`
   - 值含 `prefer-dark` → `true`
   - 值含 `prefer-light` → `false`
   - 值为 `default` → **不能判断**（GNOME <42 只返回这个），跌落到下一级
   - gsettings 命令不存在 / 超时 → 跌落下一级
2. **KDE kdeglobals**：读 `~/.config/kdeglobals` 的 `[General] ColorScheme`
   - 值名字含 "dark"（大小写不敏感）→ `true`（如 `BreezeDark` / `Oxygen-Dark` / `Breeze Dark`）
   - 值存在但不含 "dark" → `false`（已明确浅色）
   - 文件不存在或字段缺失 → 跌落下一级
3. **Qt Palette 启发式**：`QPalette::Window.lightness() < 128`（现有行为，保留作为 fallback）

### [INV-2] 超时保护

GNOME 分支调用外部 gsettings 进程，必须限制超时：
- `waitForStarted(500ms)` 和 `waitForFinished(500ms)` 各 500ms
- 超时则视为失败、跌落下一级
- 总 worst-case 开销 ≤ 1s，仅在 "follow system" 手动操作或启动时触发一次，不影响热路径

### [INV-3] QProcess 不挂在主线程阻塞 UI

本函数虽在主线程调用，但被调频率极低（用户操作或启动）。500ms × 2 超时 + 实际 gsettings 通常在 < 50ms 返回，用户不会感到明显卡顿。若未来需要更严格，可 V2 改为 QtConcurrent::run + 异步应用结果。

### [INV-4] Windows / macOS 不受影响

Linux 分支用 `#elif defined(__linux__)` 守卫，不影响 `_WIN32` 分支（Windows 继续读注册表）。其他平台（macOS 未实装）走 `#else` 分支，保持 Qt Palette 启发式。

### [INV-5] kdeglobals 字段判断不按 dark/light 精确枚举

不维护"所有 KDE 深色主题名字的白名单"——不断新增，难以穷举。
采用规则："名字小写后含子串 `dark` → 深色；其他 → 浅色"。
这对绝大多数 KDE 主题是对的；极少数命名异常的（如仅以 `Nero` 命名但实际是深色）会判错，用户可以手动切换覆盖自动检测。

### [INV-6] 检测结果**可被手动覆盖**

`applySystemTheme` 内部调本函数；用户通过菜单 视图 → 主题 显式选一款主题时，`m_currentThemeId` 非空，不再调 `isSystemDarkMode`。手动覆盖优先级高于自动检测。这是现有行为，本 Spec 不改动。

## 4. 接口

```cpp
bool MainWindow::isSystemDarkMode() const;
```

无入参，无副作用（纯函数读外部状态），可在任意位置调用。

## 5. 性能预算

| 操作 | 目标 |
|------|------|
| Windows 读注册表 | < 5 ms |
| Linux GNOME gsettings 调用（成功路径） | < 100 ms 典型 / 最多 1s 超时 |
| Linux KDE 读 ini | < 5 ms |
| Linux QPalette 回退 | < 1 ms |

## 6. 验收条件

- **[T-LDET-1]** Windows 系统设深色：`isSystemDarkMode()` 返回 true；切浅色后再调返回 false
- **[T-LDET-2]** GNOME 42+ 深色（`gsettings set ... color-scheme 'prefer-dark'`）：返回 true；切 `prefer-light` 返回 false
- **[T-LDET-3]** GNOME <42（`gsettings` 返回 `'default'`）：跌落到下一级（KDE 或 QPalette），结果取决于环境；不应 crash、不应卡死 1s 以上
- **[T-LDET-4]** KDE Plasma with `BreezeDark` scheme：返回 true；`Breeze`（浅色）返回 false
- **[T-LDET-5]** 无 gsettings 命令 / 无 kdeglobals：跌落到 QPalette 启发式，函数返回正确性依赖 Qt 已加载的系统 Palette
- **[T-LDET-6]** gsettings 进程卡住 600ms：被 500ms 超时终止，函数继续执行下一级，总耗时 < 1s
- **[T-LDET-7]** Windows 编译：Linux 分支代码被 `#elif` 排除，不产生未使用函数警告
- **[T-LDET-8]** Linux 编译：不需要额外 link library（QProcess / QDir / QSettings / QApplication 都在 Qt5::Core / Qt5::Gui / Qt5::Widgets 里，项目已 link）

## 7. 已知陷阱

### 7.1 GNOME <42 的 `default` 值

GNOME 42 才引入 `color-scheme` 字段三档值。之前版本的 gsettings 仍会返回字符串 `'default'`，但此时 GNOME 本身不区分 dark/light——只有 GTK 主题名能反映。本 Spec 不尝试解析 `gtk-theme` 字段（太容易出错），直接跌落下级。

### 7.2 KDE 主题名不含 "dark" 但实际是深色

极少数自定义主题不按约定命名。用户需要在应用菜单里手动切主题覆盖自动检测。接受此限制。

### 7.3 Wayland 不影响检测

GNOME / KDE 在 X11 和 Wayland 下都通过同一 gsettings / kdeglobals 配置存储，本检测与会话协议无关。

### 7.4 QProcess 在 snap / flatpak 沙盒下

snap / flatpak 封装的应用调 `gsettings` 可能被沙盒阻止。失败时跌落下级，不崩溃。V2 如果要强支持沙盒应用，改用 DBus 直接查询 `org.freedesktop.portal.Settings`。

### 7.5 `~/.config` 可能被 XDG_CONFIG_HOME 重定向

本 Spec 用 `QDir::homePath() + "/.config/kdeglobals"`，没尊重 `XDG_CONFIG_HOME` 环境变量。大部分用户不改默认，接受此简化。V2 可改为 `QStandardPaths::GenericConfigLocation` 遍历。

## 8. 参考

- `src/app/MainWindow.cpp::isSystemDarkMode` — 本 Spec 对应实现
- [GNOME color-scheme docs](https://developer.gnome.org/documentation/guidelines/ui/color-scheme.html)
- [KDE kdeglobals](https://userbase.kde.org/KDE_System_Administration/Configuration_Files)
- `specs/横切关注点/30-主题系统.md` — 主题切换生效机制（本 Spec 只管"决定走深色还是浅色"，不管切换后续）
