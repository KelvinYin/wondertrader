# WonderTrader 开发文档

## 1. 项目架构

### 1.1 整体架构
WonderTrader采用模块化设计，主要包含以下核心组件：
- **BaseLibs**: 基础组件库
- **DataKit**: 数据组件
- **Backtest**: 回测系统
- **Parsers**: 行情解析器
- **Traders**: 交易接口

### 1.2 核心模块说明

#### BaseLibs
- Share: 包含基础数据结构、对象定义和接口定义
- WTSUtilsLib: 第三方组件封装(pugixml、zstdlib等)
- WTSToolsLib: 通用工具库(日志、基础数据管理等)

#### DataKit
- WtDtCore: 数据组件核心库
- WtDtPorter: 数据组件C接口导出库
- WtDataStorage: 数据读取组件
- WtDataStorageAD: 数据落地组件
- WtDtHelper: 数据辅助工具
- WtDtServo: 数据服务

#### Backtest
- WtBtCore: 回测框架核心代码
- WtBtPorter: 回测框架C接口导出模块
- WtBtRunner: 回测框架C++运行入口

## 2. 跨平台支持

### 2.1 Windows
- 开发环境: Visual Studio 2017
- 操作系统: Windows 10及以上
- 编译工具: MSVC

### 2.2 Linux
- 开发环境: gcc v8.4.0
- 构建工具: cmake 3.17.5
- 支持主流Linux发行版

### 2.3 macOS
- 开发环境: Xcode Command Line Tools
- 构建工具: cmake 3.17.5
- 支持macOS 10.15及以上版本

#### 依赖库安装位置
- Homebrew安装的依赖库位于 `/usr/local/opt/` 或 `/opt/homebrew/opt/`(Apple Silicon)
  - boost: `/usr/local/opt/boost/` 或 `/opt/homebrew/opt/boost/`
  - nanomsg: `/usr/local/opt/nanomsg/` 或 `/opt/homebrew/opt/nanomsg/`
  - rapidjson: `/usr/local/opt/rapidjson/` 或 `/opt/homebrew/opt/rapidjson/`
  - spdlog: `/usr/local/opt/spdlog/` 或 `/opt/homebrew/opt/spdlog/`
- 头文件目录: `include/`
- 库文件目录: `lib/`

注意：在CMake配置时，需要通过以下方式指定依赖库路径：
```bash
# Intel Mac
cmake .. -DBOOST_ROOT=/usr/local/opt/boost

# Apple Silicon Mac
cmake .. -DBOOST_ROOT=/opt/homebrew/opt/boost
```

### 2.4 依赖库
- boost 1.72
- rapidjson 1.0.2
- spdlog 1.9.2
- nanomsg 1.1.5

## 3. 编译指南

### 3.1 Windows编译
1. 使用Visual Studio 2017打开对应解决方案：
   - all.sln: 编译所有模块
   - backtest.sln: 仅编译回测系统
   - datakit.sln: 仅编译数据组件
   - parsers.sln: 仅编译行情解析器
   - traders.sln: 仅编译交易接口
   - uft.sln: 仅编译UFT相关模块

2. 选择编译配置(Debug/Release)和平台(x86/x64)

3. 执行编译

### 3.2 Linux编译
1. 安装依赖：
```bash
# 安装编译工具
sudo apt-get install build-essential cmake

# 安装依赖库
sudo apt-get install libboost-all-dev
```

2. 编译项目：
```bash
cd src
mkdir build && cd build
cmake ..
make
```

### 3.3 macOS编译
1. 安装依赖：
```bash
# 安装Xcode Command Line Tools
xcode-select --install

# 使用Homebrew安装依赖库
brew install boost
brew install cmake
brew install nanomsg
```

2. 编译项目：
```bash
cd src
mkdir build && cd build
cmake ..
make
```

## 4. 核心功能实现

### 4.1 交易接口适配
- CTP接口
  - ParserCTP: CTP行情接口适配
  - TraderCTP: CTP交易接口适配
  - 支持多版本CTP API(6.3.15/6.6.9)
- OES接口
  - ParserOES: OES行情接口适配
  - TraderOES: OES交易接口适配
- XTP接口
  - ParserXTP: XTP行情接口适配
  - TraderXTP: XTP交易接口适配
- 其他接口
  - ParserFemas: 飞马行情接口适配
  - ParserXeleSkt: 艾克朗科组播行情接口适配
  - ParserYD: 易达期货行情接口适配

### 4.2 数据存储
- 采用自定义二进制格式存储行情数据
- 支持Tick数据和K线数据存储
- 提供高效的数据读取接口
- 支持LMDB数据存储引擎

### 4.3 回测系统
- 支持CTA策略回测
- 支持高频策略回测
- 支持UFT策略回测
- 提供详细的回测报告和分析工具
- 支持多合约组合策略回测

### 4.4 实盘交易
- 支持CTA策略实盘
- 支持高频策略实盘
- 支持UFT策略实盘
- 提供风控模块
- 支持多账户交易

### 4.5 WTSTrader交易界面
- 基于FLTK实现的跨平台交易界面
- 支持账户管理、持仓查看、委托管理等功能
- 提供实时行情展示和深度行情分析
- 集成策略管理和回测分析功能
- 支持风控监控和交易管理

#### 编译依赖
- FLTK 1.3.5及以上版本
- 支持Windows、Linux和macOS平台

#### 编译说明
1. Windows平台
```bash
# 安装FLTK依赖
# 使用Visual Studio 2017打开WTSTrader项目编译
```

2. Linux平台
```bash
# 安装FLTK开发包
sudo apt-get install libfltk1.3-dev

# 编译WTSTrader
cd src/WTSTrader
mkdir build && cd build
cmake ..
make
```

3. macOS平台
```bash
# 使用Homebrew安装FLTK
brew install fltk

# 编译WTSTrader
cd src/WTSTrader
mkdir build && cd build
cmake ..
make
```

## 5. API接口

### 5.1 C接口
- WtBtPorter: 回测系统接口
- WtDtPorter: 数据系统接口
- WtPorter: 实盘交易接口
- WtExecMon: 执行管理接口

### 5.2 Python封装
- wtpy: Python语言封装
- 支持策略开发
- 支持数据处理
- 支持回测和实盘
- 提供完整的策略开发框架