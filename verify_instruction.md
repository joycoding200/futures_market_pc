# 期货行情PC软件 — 编译与验证指南

## 1. 环境准备

### 1.1 安装 Qt 6.x

通过 [Qt Online Installer](https://www.qt.io/download-qt-installer) 安装：
- 选择 **Qt 6.x → MSVC 2019/2022 64-bit**（Windows）
- Linux/macOS 用户选择对应平台的 Qt 6.x

### 1.2 安装 CMake

```bash
# Windows: https://cmake.org/download/
# Linux: sudo apt install cmake
# macOS: brew install cmake
```

### 1.3 安装 vcpkg 及三方库

```bash
# 克隆 vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:/vcpkg
cd C:/vcpkg
./bootstrap-vcpkg.bat

# 安装依赖库
vcpkg install spdlog:x64-windows
vcpkg install sqlite3:x64-windows
vcpkg install qcustomplot:x64-windows
```

### 1.4 CTP SDK

`third_party/ctp/` 目录已包含 CTP 行情 SDK，无需额外下载。

运行前将 dll 拷贝到可执行文件同目录：
```bash
copy third_party\ctp\thostmduserapi_se.dll build\Release\
```

## 2. 编译

```bash
cd pc_futuresmarketanalysis
mkdir build && cd build

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2019_64

cmake --build . --config Release
```

编译产物：`build/Release/FuturesMarketPC.exe`

## 3. 运行

```bash
cd build/Release
./FuturesMarketPC.exe
```

程序默认使用**模拟数据源**启动，无需额外配置即可查看行情展示效果。

## 4. 功能验证

| # | 验证项 | 操作 | 预期结果 |
|---|--------|------|----------|
| 1 | 主界面 | 启动程序 | 三栏布局：左侧合约列表 / 中央K线图+指标 / 右侧盘口+成交 |
| 2 | 模拟行情 | 默认启动（Sim数据源） | K线图实时更新，盘口数据变化，分时线右移 |
| 3 | K线缩放 | 鼠标滚轮 | K线根数增减，坐标自适应 |
| 4 | K线平移 | 鼠标左键拖拽 | 图表左右平移 |
| 5 | 十字光标 | 鼠标悬停K线 | 十字线显示，浮窗显示O/H/L/C/V |
| 6 | 合约切换 | 点击左侧合约列表 | K线/分时/盘口同步切换 |
| 7 | MA指标 | 观察K线主图 | 叠加4条均线(MA5白/MA10黄/MA20紫/MA60蓝) |
| 8 | MACD副图 | K线下方副图 | 红绿柱+DIF白线+DEA黄线 |
| 9 | 盘口面板 | 右侧上方面板 | 五档买卖盘+最新价+涨跌幅+量仓 |
| 10 | 分时图 | 下方Tab切换 | 价格折线+均价虚线+昨结算基线 |
| 11 | 系统设置 | 菜单 系统→系统设置 | 弹出设置对话框（行情+数据源标签） |
| 12 | 刷新率修改 | 修改刷新间隔→确定 | Config持久化保存 |
| 13 | 合约管理 | 菜单 系统→合约管理 | 弹出管理对话框，可添加/双击删除合约 |
| 14 | 关于 | 菜单 系统→关于 | 显示版本号、Qt版本、技术栈 |

## 5. CTP 真实行情验证

需期货公司提供的 CTP 行情账号：

1. 打开 **系统 → 系统设置 → 数据源** 标签
2. 设置行情前置地址（如 `tcp://180.168.146.187:10131`）
3. 设置 BrokerID
4. 选择数据源类型为 **CTP** → 确定
5. **重启程序**（数据源在启动时选择）

预期日志输出：
```
CTPDataProvider: 连接 ...
CTP: 前置连接成功
CTP: 登录成功
CTP: 订阅 rb2510
```

K线图和盘口将显示真实期货行情数据。
