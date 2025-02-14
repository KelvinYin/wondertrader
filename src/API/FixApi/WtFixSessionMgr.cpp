#include "WtFixSessionMgr.h"
#include "WtFixSession.h"
#include <sstream>

WtFixSessionMgr::WtFixSessionMgr() {}

WtFixSessionMgr::~WtFixSessionMgr() {
    _sessions.clear();
}

std::shared_ptr<WtFixSession> WtFixSessionMgr::createSession(
    const std::string& senderCompId,
    const std::string& targetCompId,
    const std::string& version) {
    
    // 生成会话键值
    std::string sessionKey = senderCompId + ":" + targetCompId;
    
    // 检查会话是否已存在
    auto it = _sessions.find(sessionKey);
    if (it != _sessions.end()) {
        return it->second;
    }
    
    // 创建新会话
    auto session = std::make_shared<WtFixSession>(senderCompId, targetCompId);
    session->setVersion(version);
    
    // 设置回调函数
    if (_msgCallback) {
        session->setMsgCallback(_msgCallback);
    }
    if (_logCallback) {
        session->setLogCallback(_logCallback);
    }
    
    // 保存会话
    _sessions[sessionKey] = session;
    
    // 更新统计信息
    _stats.totalMsgs = 0;
    _stats.errorMsgs = 0;
    _stats.avgProcessTime = 0.0;
    _stats.lastMsgTime = std::chrono::system_clock::now();
    
    return session;
}

std::shared_ptr<WtFixSession> WtFixSessionMgr::getSession(
    const std::string& senderCompId,
    const std::string& targetCompId) {
    
    std::string sessionKey = senderCompId + ":" + targetCompId;
    auto it = _sessions.find(sessionKey);
    
    return (it != _sessions.end()) ? it->second : nullptr;
}

void WtFixSessionMgr::removeSession(
    const std::string& senderCompId,
    const std::string& targetCompId) {
    
    std::string sessionKey = senderCompId + ":" + targetCompId;
    _sessions.erase(sessionKey);
}