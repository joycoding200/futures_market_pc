# 构建踩坑记录与经验总结

## 踩坑清单

### 坑1：Qt 路径不透明

**现象**：CMake 找不到 Qt6。
**原因**：Qt 安装在 `C:/Dev/Qt/6.11.0/mingw_64`，非标准路径。搜索了大量路径（C:/Qt, D:/Qt, Program Files, conda）才找到。
**教训**：首次构建前应直接询问用户 Qt 安装路径，或让用户在环境变量中设置 `CMAKE_PREFIX_PATH`。

### 坑2：vcpkg 网络不可用

**现象**：vcpkg install 失败，无法下载 PowerShell-Core。
**原因**：网络环境限制（GFW），GitHub 下载被阻断。
**教训**：不应依赖 vcpkg 在线安装。应优先使用已安装的 Qt6（自带 SQLite）和本地编译的依赖。

### 坑3：CMakeLists.txt 写了未使用的依赖

**现象**：`find_package(spdlog REQUIRED)`、`find_package(SQLite3 REQUIRED)`、`find_package(qcustomplot REQUIRED)` 全部失败。
**原因**：
- `spdlog`：未通过 vcpkg/系统包管理器安装
- `SQLite3`：Qt6::Sql 已内置 QSQLITE 驱动，不需要独立 SQLite
- `qcustomplot`：项目代码中**从未使用**，纯冗余依赖
**教训**：CMakeLists.txt 中只声明真正使用的依赖。Qt6 自带 SQLite 支持。

**解决方案**：
```cmake
# 删除
find_package(SQLite3 REQUIRED)    # Qt6::Sql 已提供
find_package(qcustomplot REQUIRED) # 未使用

# spdlog 改用本地源码
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/spdlog ${CMAKE_BINARY_DIR}/spdlog)
endif()
```

### 坑4：CTP 头文件无条件包含

**现象**：`WITH_CTP=OFF` 时编译失败 `ThostFtdcMdApi.h: No such file or directory`。
**原因**：`CTPDataProvider.cpp` 第4行 `#include <ThostFtdcMdApi.h>` 没有条件编译保护。
**教训**：条件编译的可选功能必须从源码头文件到实现代码全程使用 `#ifdef` 保护。

**解决方案**：
```cpp
#ifdef WITH_CTP
#include <ThostFtdcMdApi.h>
#endif
```

### 坑5：缺少 `#include <QDateTime>`

**现象**：`incomplete type 'QDateTime' used in nested name specifier`。
**原因**：`SimDataProvider.cpp` 和 `KLineAggregator.cpp` 中使用了 `QDateTime::currentMSecsSinceEpoch()` 和 `QDateTime::fromMSecsSinceEpoch()`，但没有 `#include <QDateTime>`。
**教训**：Qt5 中某些头文件隐式包含了 QDateTime，Qt6 中更严格。每个文件必须显式包含自己所使用的所有 Qt 类头文件。

**需要显式包含 `<QDateTime>` 的文件**：
- `src/data/SimDataProvider.cpp`
- `src/data/KLineAggregator.cpp`
- `src/data/CTPDataProvider.cpp`

### 坑6：类型硬编码 `SimDataProvider*` 应为 `MarketDataProvider*`

**现象**：`cannot convert 'CTPDataProvider*' to 'SimDataProvider*'`。
**原因**：`Application.h` 和 `MainWindow.h` 中将 `m_dataProvider` 声明为 `SimDataProvider*`，但后续扩展了 CTPDataProvider。
**教训**：多态场景下，成员变量应声明为**基类指针**（`MarketDataProvider*`），而非具体实现类。

**修复**：
```cpp
// 错误
class SimDataProvider;  // 前向声明
SimDataProvider* m_dataProvider;

// 正确
class MarketDataProvider;  // 前向声明基类
MarketDataProvider* m_dataProvider;
```

涉及文件：
- `src/app/Application.h` — 成员类型 + 前向声明
- `src/app/Application.cpp` — 信号连接使用基类信号 `&MarketDataProvider::tickReceived`
- `src/app/MainWindow.h` — 构造函数参数 + 成员类型 + 前向声明
- `src/app/MainWindow.cpp` — include 改为 `MarketDataProvider.h`

### 坑7：GCC13 类型检查比 MSVC 更严格

**现象**：`std::max(0, klines.size() - m_visibleCount)` 编译失败。
**原因**：`0` 是 `int`，`klines.size()` 返回 `qsizetype`（GCC 上是 `long long int`）。`std::max` 要求两个参数类型一致。MSVC 宽松处理，GCC13 拒绝。
**教训**：不要混用 `int` 和 `qsizetype`。

**修复**：
```cpp
// 错误
std::max(0, klines.size() - m_visibleCount)

// 正确
std::max<int>(0, static_cast<int>(klines.size()) - m_visibleCount)
```

### 坑8：Qt6 中 `QTreeWidgetItem::setTextColor()` 已移除

**现象**：`'class QTreeWidgetItem' has no member named 'setTextColor'`。
**原因**：Qt5 → Qt6 API 变更。
**修复**：替换为 `setForeground()`。

### 坑9：Windows 上 build 目录被锁定

**现象**：`rm -rf build` 失败 "Device or resource busy"。
**原因**：Windows 上 mingw32-make 或 cmake 进程可能仍持有 build 目录中的文件句柄。
**教训**：清理构建前确保所有构建进程已退出。使用新目录名（如 build2, bld）重建比清理旧目录更可靠。

## 快速构建检查清单

在首次构建前，确认以下事项：

- [ ] Qt6 安装路径已知：`C:/Dev/Qt/6.11.0/mingw_64`
- [ ] MinGW 路径已知：`C:/Dev/Qt/Tools/mingw1310_64/bin`
- [ ] spdlog 已放入 `third_party/spdlog/`
- [ ] CMakeLists.txt 无冗余依赖（SQLite3/qcustomplot 已移除）
- [ ] 所有 `.cpp` 文件显式包含所需的 Qt 头文件（特别是 `<QDateTime>`）
- [ ] 多态成员变量使用基类指针（`MarketDataProvider*`，非 `SimDataProvider*`）
- [ ] `std::max` 参数类型一致（必要时 `static_cast<int>`）
- [ ] Qt5 API（`setTextColor`）已更新为 Qt6 API（`setForeground`）
- [ ] 可选功能（CTP）使用 `#ifdef` 保护
- [ ] CMake 使用 `-G "MinGW Makefiles"`（非默认的 Visual Studio generator）

---

## Phase 5 构建经验（2026-05-12）

### 坑10：build.sh 手写 g++ 编译缺少 DLL 部署

**现象**：`bin/FuturesMarketPC.exe` 编译成功但双击报错 `找不到 Qt6Core.dll`。
**原因**：手写 `g++ -L -l` 只能解决链接时依赖，运行时 Windows 不会自动搜索 Qt 的 bin 目录。
**教训**：Qt 应用的 Windows 部署必须使用 `windeployqt` 工具，它会自动分析 exe 依赖并拷贝所需 DLL + 平台插件（`platforms/qwindows.dll` 是必须的，否则连窗口都创建不了）。

**解决方案**（终极）：回归 CMake + windeployqt。

### 坑11：CMakeLists.txt 缺少 MainWindow.cpp

**现象**：CMake 链接报 `undefined reference to MainWindow::MainWindow`。
**原因**：`src/app/MainWindow.cpp` 漏写在 `SOURCES` 列表中。手写 build.sh 中包含了它，迁移到 CMake 时遗留。
**教训**：CMakeLists.txt 的 SOURCES 应与实际文件严格同步。新增 `.cpp` 时记得加进去。

### 坑12：CMake spdlog 集成方式

**现象**：vcpkg 网络不通，无法通过 `find_package(spdlog)` 找到。
**解决方案**：
```cmake
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/spdlog ${CMAKE_BINARY_DIR}/spdlog)
endif()
```
spdlog 放在 `third_party/spdlog/` 目录，自带 CMakeLists.txt，作为子项目编译即可。无需 vcpkg。

### 正确的 CMake 构建命令

```bash
export PATH="/c/Dev/Qt/Tools/mingw1310_64/bin:/c/Dev/Qt/6.11.0/mingw_64/bin:$PATH"
cmake -G "MinGW Makefiles" \
    -DCMAKE_PREFIX_PATH=C:/dev/Qt/6.11.0/mingw_64 \
    -DCMAKE_BUILD_TYPE=Release \
    -B build_cmake -S .
cmake --build build_cmake
windeployqt build_cmake/FuturesMarketPC.exe --release --no-translations
```

关键参数：
- `-G "MinGW Makefiles"` — 必须，Windows 默认是 Visual Studio generator
- `-DCMAKE_PREFIX_PATH=...` — 告诉 CMake 去哪找 Qt6
- `windeployqt --no-translations` — 跳过翻译文件减小体积
- MinGW 的 bin 和 Qt 的 bin **都要在 PATH** 中（CMake 需要 g++，windeployqt 需要 Qt DLL）
