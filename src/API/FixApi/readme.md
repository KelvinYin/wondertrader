# FIX协议接入模块

## 接口介绍
本接口实现了标准的FIX（Financial Information eXchange）协议，支持FIX 5.0版本。作为一个专业的金融交易协议接入模块，它提供了完整的FIX协议支持，可用于连接各类支持FIX协议的交易所和经纪商系统。

## 功能特点
1. 支持标准FIX协议消息的编解码
   - 支持所有标准FIX消息类型
   - 支持自定义消息字段扩展
   - 提供高效的消息序列化和反序列化

2. 完整的FIX会话管理
   - 自动处理登录认证
   - 维护心跳机制
   - 支持序号同步和重传
   - 自动处理会话重连

3. 灵活的消息处理机制
   - 支持同步和异步消息处理
   - 提供消息过滤和转换接口
   - 支持自定义消息处理回调

4. 高性能设计
   - 采用异步IO模型
   - 支持消息批量处理
   - 内置高效的消息缓存机制

## 使用说明
1. 编译要求
   - 支持32位和64位系统编译
   - 建议使用Release模式以获得最佳性能
   - 确保系统已安装必要的网络库

2. 运行环境要求
   - 建议使用专线网络以确保稳定性
   - 配置足够的系统资源
   - 确保网络防火墙允许FIX协议端口

3. 基本使用流程
   - 配置会话参数
   - 初始化FIX会话
   - 注册消息处理回调
   - 建立连接并维持会话

## 配置说明
1. 必要配置项
   - BeginString: FIX协议版本，如"FIX.5"
   - SenderCompID: 发送方标识，用于身份识别
   - TargetCompID: 接收方标识，用于对手方识别
   - HeartBtInt: 心跳间隔（秒），建议设置为30
   - ResetSeqNumFlag: 是否重置序号标志

2. 可选配置项
   - DataDictionary: 数据字典文件路径，用于自定义消息定义
   - StartTime: 交易时段开始时间，格式如"09:00:00"
   - EndTime: 交易时段结束时间，格式如"15:00:00"
   - SocketConnectHost: 连接地址
   - SocketConnectPort: 连接端口
   - LogonTimeout: 登录超时时间（秒）
   - ReconnectInterval: 重连间隔（秒）

3. 性能相关配置
   - SendBufferSize: 发送缓冲区大小（KB）
   - ReceiveBufferSize: 接收缓冲区大小（KB）
   - ValidateFieldsOutOfOrder: 是否校验字段顺序
   - ValidateFieldsHaveValues: 是否校验字段值

## 使用示例
```yaml
# config.yaml示例
fixTest:
    BeginString: "FIX.5.0"
    SenderCompID: "CLIENT1"
    TargetCompID: "BROKER"
    HeartBtInt: 30
    ResetSeqNumFlag: true
    DataDictionary: "./config/FIX5.xml"
    StartTime: "09:00:00"
    EndTime: "15:00:00"
    SocketConnectHost: "127.0.0.1"
    SocketConnectPort: 5001
```

## 注意事项
1. 确保配置文件中的SenderCompID和TargetCompID与对手方配置匹配
2. 建议在正式环境中使用专线网络以确保连接稳定性
3. 根据实际需求调整心跳间隔和重连参数
4. 建议启用消息校验以确保数据完整性
5. 定期检查日志以监控连接状态和消息处理情况
