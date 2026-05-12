# 关注事项与隐患

## Critical (0)

无已知严重问题。

## Medium (4)

| # | 问题 | 位置 | 影响 | 建议 |
|---|------|------|------|------|
| C1 | **无正式测试框架** | `tests/` | 回归风险高，无法保障代码质量 | 引入 Google Test/Catch2，集成 CMake CTest |
| C2 | **vcpkg 路径硬编码** | `CMakeLists.txt`, `verify_instruction.md` | 其他开发者无法直接构建 | 使用环境变量 `VCPKG_ROOT` 替代硬编码路径 |
| C3 | **requestHistory 未实现** | `SimDataProvider::requestHistory()` | 模拟模式下无历史 K 线数据 | 实现历史数据生成逻辑或从文件加载 |
| C4 | **无 CI/CD** | 项目根目录 | 无自动构建/测试流水线 | 添加 GitHub Actions (Windows + MSVC) |

## Low (3)

| # | 问题 | 位置 | 影响 | 建议 |
|---|------|------|------|------|
| C5 | **裸指针大量使用** | `Application.h`, `MainWindow.h` | 生命周期依赖外部管理 | 考虑使用 `std::unique_ptr` 或 `QPointer` |
| C6 | **KLineAggregator 固定数组** | `KLineAggregator.h:23` | 新增周期需改数组大小 | 使用 `QMap<KLineType, KLineData>` 替代 `[8]` |
| C7 | **build 目录残留** | 项目根目录 | 多份 build 占用磁盘 | 清理并统一为一个 `build/` 目录 |
| C8 | **vcpkg_installed 在项目内** | `vcpkg_installed/` | 可能意外提交，体积大 | 确认 `.gitignore` 已排除 |

## 已修复 (10)

参见 `docs/review-report.md`：Phase 1-4 全部 10 个问题已修复（1 Critical + 3 Medium + 6 Low）。

## 技术债总结

当前项目处于**功能完整**阶段，主要技术债集中在：
1. **测试** — 最紧迫，3 个 assert 骨架无法支撑后续开发
2. **可移植性** — 硬编码路径和 Windows 专属配置限制了跨平台能力
3. **自动化** — 无 CI/CD，依赖手动构建和验证
