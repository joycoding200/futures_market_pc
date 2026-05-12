# 项目状态

> 最后更新: 2026-05-11

## 当前阶段

Phase 1-5 全部完成。

## 完成情况

- Phase 1 (行情展示): ✅ K线图/分时图/盘口面板/合约列表/十字光标
- Phase 2 (技术指标): ✅ MA/MACD/KDJ/RSI，支持 Overlay 和 SubChart
- Phase 3 (CTP 接入): ✅ CTPDataProvider，条件编译，数据源切换
- Phase 4 (系统管理): ✅ 设置对话框/合约管理/关于对话框
- Phase 5 (数据统计与筛选): ✅ StatsCalculator/StatsPanel/成交量分布/合约筛选

## 已知问题

- 无正式测试框架（3 个 assert 骨架）
- vcpkg 路径硬编码
- requestHistory 未实现
- 无 CI/CD 配置
- 缺少用户使用说明文档

## 下一步

1. 编写用户使用说明文档（README.md）
2. 测试体系建设 + CI/CD 集成
