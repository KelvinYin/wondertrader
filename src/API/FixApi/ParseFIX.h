#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>

class ParseFIX {
public:
    ParseFIX();
    ~ParseFIX();

    // 解析FIX消息
    bool parse(const std::string& msg, std::map<int, std::string>& fields);

    // 验证必填字段
    bool validateRequiredFields(const std::map<int, std::string>& fields);

    // 设置和获取FIX版本
    void setVersion(const std::string& ver) { _version = ver; }
    const std::string& getVersion() const { return _version; }

    // 设置日志回调
    void setLogCallback(std::function<void(const std::string&)> callback) { _logCallback = callback; }

private:
    // 解析消息头
    bool parseHeader(const std::string& msg, size_t& pos, std::map<int, std::string>& fields);

    // 解析消息体
    bool parseBody(const std::string& msg, size_t& pos, std::map<int, std::string>& fields);

    // 验证校验和
    bool validateChecksum(const std::string& msg, const std::string& checksum);

    // 记录日志
    void log(const std::string& msg);

    std::string _version;                                      // FIX版本
    std::function<void(const std::string&)> _logCallback;      // 日志回调函数
    static const char FIELD_SEPARATOR = 1;                    // 字段分隔符
    static const char KEY_VALUE_SEPARATOR = '=';              // 键值分隔符
};