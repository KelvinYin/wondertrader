#include "WtFixApiBase.h"
#include "WtFixMsgCodec.h"
#include <sstream>

void WtFixApiBase::handleMessage(const std::string& msg) {
    // 解析FIX消息
    std::map<int, std::string> fields;
    if (!WtFixMsgCodec::decode(msg, fields)) {
        if (_logCallback) {
            _logCallback("Failed to decode FIX message: " + msg);
        }
        return;
    }

    // 调用消息回调
    if (_msgCallback) {
        _msgCallback(fields);
    }
}

bool WtFixApiBase::sendMessage(const std::string& msg) {
    if (!_connected) {
        if (_logCallback) {
            _logCallback("Not connected to FIX server");
        }
        return false;
    }

    // 实际的发送逻辑由子类实现
    return true;
}