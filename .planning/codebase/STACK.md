# 技术栈

## 语言与运行时

| 类别 | 选型 | 版本 |
|------|------|------|
| 语言 | C++17 | - |
| UI 框架 | Qt 6.x (Core/Gui/Widgets/Sql) | 6.x |
| 构建系统 | CMake | 3.22+ |
| 包管理 | vcpkg | - |

## 三方库

| 库 | 用途 | 集成方式 |
|----|------|----------|
| spdlog | 日志 | CMake find_package + FetchContent fallback |
| SQLite | 数据持久化 | Qt6::Sql 内置驱动 |
| CTP SDK | 真实行情接入 | 条件编译 WITH_CTP，动态链接 `thostmduserapi_se` |

## 开发工具

| 工具 | 用途 |
|------|------|
| Qt Online Installer | Qt 框架安装 |
| CMake | 跨平台构建 |
| vcpkg | C++ 包管理 |
| Visual Studio 2019/2022 | Windows 编译 (MSVC) |

## 依赖注意事项

- `vcpkg.json` 中声明了 `qcustomplot`，但 CMakeLists.txt 中未实际链接（死依赖）
- CTP SDK 的 DLL 需手动拷贝到可执行文件同目录
- spdlog 使用 header-only 模式，通过 FetchContent 自动拉取
