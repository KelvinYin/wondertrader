#pragma once
#include <string>
#include <map>
#include <memory>
#include <functional>
#include "WtFixMsgCodec.h"

class WtFixSession;

// FIX会话管理器
class WtFixSessionMgr {
public:
    WtFixSessionMgr();
    ~WtFixSessionMgr();

    // 创建新的FIX会话
    std::shared_ptr<WtFixSession> createSession(const std::string& senderCompId,
                                               const std::string& targetCompId,
                                               const std::string& version);

    // 根据发送方和接收方ID获取会话
    std::shared_ptr<WtFixSession> getSession(const std::string& senderCompId,
                                            const std::string& targetCompId);

    // 移除会话
    void removeSession(const std::string& senderCompId,
                      const std::string& targetCompId);

    // 设置消息处理回调
    void setMsgCallback(std::function<void(const std::map<int, std::string>&)> callback) {
        _msgCallback = callback;
    }

    // 设置日志回调
    void setLogCallback(std::function<void(const std::string&)> callback) {
        _logCallback = callback;
    }

    // 获取会话统计信息
    const FixMsgStats& getStats() const { return _stats; }

    // 重置统计信息
    void resetStats() { _stats = FixMsgStats(); }

private:
    // 会话映射表，key为"senderCompId:targetCompId"
    std::map<std::string, std::shared_ptr<WtFixSession>> _sessions;
    
    // 消息统计信息
    FixMsgStats _stats;

    // 回调函数
    std::function<void(const std::map<int, std::string>&)> _msgCallback;
    std::function<void(const std::string&)> _logCallback;
};