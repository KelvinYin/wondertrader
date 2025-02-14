#pragma once
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <functional>
#include <memory>

// 前向声明
class IFixMsgHandler;
class IFixMsgFilter;

// 消息统计信息结构
struct FixMsgStats {
    uint64_t totalMsgs{0};        // 总消息数
    uint64_t errorMsgs{0};        // 错误消息数
    double avgProcessTime{0.0};    // 平均处理时间(ms)
    std::chrono::system_clock::time_point lastMsgTime;  // 最后一条消息时间
};

class WtFixMsgCodec {
public:
    WtFixMsgCodec();
    ~WtFixMsgCodec();

    // 编码FIX消息
    std::string encode(const std::map<int, std::string>& fields);

    // 解码FIX消息
    bool decode(const std::string& msg, std::map<int, std::string>& fields);

    // 设置FIX版本
    void setVersion(const std::string& ver) { _version = ver; }

    // 获取FIX版本
    const std::string& getVersion() const { return _version; }

    // 监控和调试功能
    void enableDebug(bool enable) { _debugEnabled = enable; }
    void setLogCallback(std::function<void(const std::string&)> callback) { _logCallback = callback; }
    const FixMsgStats& getStats() const { return _stats; }
    void resetStats() { _stats = FixMsgStats(); }

    // 消息处理器和过滤器管理
    void addMsgHandler(std::shared_ptr<IFixMsgHandler> handler);
    void addMsgFilter(std::shared_ptr<IFixMsgFilter> filter);
    void clearHandlers() { _handlers.clear(); }
    void clearFilters() { _filters.clear(); }


private:
    // 计算校验和
    std::string calculateChecksum(const std::string& msg);

    // 验证校验和
    bool validateChecksum(const std::string& msg, const std::string& checksum);

    // 日志记录
    void logMessage(const std::string& msg);
    
    // 更新统计信息
    void updateStats(bool success, double processTime);

    std::string _version;         // FIX版本
    std::string _beginString;     // FIX传输协议版本
    std::string _applVersion;     // FIX应用层版本
    char _delimiter;              // 字段分隔符
    bool _isFixt;                 // 是否使用FIXT传输协议
    bool _debugEnabled{false};    // 调试模式开关

    // 监控和调试相关成员
    FixMsgStats _stats;           // 消息统计信息
    std::function<void(const std::string&)> _logCallback;  // 日志回调函数

    // 消息处理器和过滤器
    std::vector<std::shared_ptr<IFixMsgHandler>> _handlers;
    std::vector<std::shared_ptr<IFixMsgFilter>> _filters;

    // FIXT.1.1特定字段
    static const int APPL_VER_ID = 1128;     // DefaultApplVerID字段
    static const int MSG_TYPE = 35;          // MsgType字段
    static const int BEGIN_STRING = 8;       // BeginString字段
    static const int BODY_LENGTH = 9;        // BodyLength字段
    static const int SENDING_TIME = 52;      // SendingTime字段
    static const int CHECKSUM = 10;          // Checksum字段
};