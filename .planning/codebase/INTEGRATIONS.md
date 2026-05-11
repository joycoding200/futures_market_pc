# 集成点分析

## 外部依赖

| 依赖 | 集成方式 | 路径/配置 | 状态 |
|------|----------|-----------|------|
| Qt 6.x | CMake `find_package` | `CMAKE_PREFIX_PATH` | ✅ |
| spdlog | `find_package` + `FetchContent` fallback | `third_party/spdlog/` | ✅ |
| SQLite | Qt6::Sql 内置驱动 | - | ✅ |
| CTP SDK | 条件编译 `WITH_CTP` | `third_party/ctp/` | ⚠️ |
| vcpkg | CMake Toolchain | `C:/vcpkg/` (硬编码) | ⚠️ |

## CTP 集成细节

```
CMakeLists.txt:
  option(WITH_CTP "Build with CTP support" OFF)
  if(WITH_CTP):
    target_include_directories → third_party/ctp/
    target_link_libraries → thostmduserapi_se
    target_compile_definitions → WITH_CTP

Application.cpp:
  #ifdef WITH_CTP
    m_dataProvider = new CTPDataProvider()
  #else
    m_dataProvider = new SimDataProvider()
  #endif
```

## 运行时依赖

| 文件 | 来源 | 说明 |
|------|------|------|
| `thostmduserapi_se.dll` | `third_party/ctp/` | CTP 行情 DLL，需手动拷贝到 exe 同目录 |
| Qt6*.dll | Qt 安装目录 | Qt 运行时库 |
| `Qt6/sqlite3` | Qt 插件 | SQLite 驱动插件 |

## API 集成

| 接口 | 协议 | 说明 |
|------|------|------|
| CTP 行情前置 | TCP (自定义协议) | 需期货公司提供地址/端口 |
| 配置持久化 | QSettings | 本地文件存储（INI/注册表） |
| 数据库 | SQLite | 本地文件数据库 |

## 已知风险

1. **vcpkg 路径硬编码**：`C:/vcpkg/` 要求 vcpkg 必须安装在 C 盘根目录
2. **CTP 条件编译**：默认 OFF，发布版本需手动启用
3. **DLL 手动部署**：CTP DLL 不会自动拷贝到输出目录
4. **无跨平台验证**：当前仅在 Windows 上测试（MSVC 2019/2022）
