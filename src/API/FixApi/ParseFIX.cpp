#include "ParseFIX.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

ParseFIX::ParseFIX() : _version("FIX.5.0") {}

ParseFIX::~ParseFIX() {}

bool ParseFIX::parse(const std::string& msg, std::map<int, std::string>& fields) {
    if (msg.empty()) {
        log("Empty message received");
        return false;
    }

    size_t pos = 0;
    // 解析消息头
    if (!parseHeader(msg, pos, fields)) {
        log("Failed to parse message header");
        return false;
    }

    // 解析消息体
    if (!parseBody(msg, pos, fields)) {
        log("Failed to parse message body");
        return false;
    }

    // 验证校验和
    auto it = fields.find(10); // 10是校验和字段
    if (it == fields.end()) {
        log("Missing checksum");
        return false;
    }

    if (!validateChecksum(msg, it->second)) {
        log("Invalid checksum");
        return false;
    }

    return true;
}

bool ParseFIX::parseHeader(const std::string& msg, size_t& pos, std::map<int, std::string>& fields) {
    while (pos < msg.length()) {
        size_t nextSep = msg.find(FIELD_SEPARATOR, pos);
        if (nextSep == std::string::npos) break;

        std::string field = msg.substr(pos, nextSep - pos);
        size_t eqPos = field.find(KEY_VALUE_SEPARATOR);
        if (eqPos == std::string::npos) {
            log("Invalid field format in header");
            return false;
        }

        int tag = std::stoi(field.substr(0, eqPos));
        std::string value = field.substr(eqPos + 1);
        fields[tag] = value;

        // 如果已经处理完消息头（35字段是消息类型，标志着消息头的结束）
        if (tag == 35) {
            pos = nextSep + 1;
            return true;
        }

        pos = nextSep + 1;
    }

    log("Incomplete header");
    return false;
}

bool ParseFIX::parseBody(const std::string& msg, size_t& pos, std::map<int, std::string>& fields) {
    while (pos < msg.length()) {
        size_t nextSep = msg.find(FIELD_SEPARATOR, pos);
        if (nextSep == std::string::npos) break;

        std::string field = msg.substr(pos, nextSep - pos);
        size_t eqPos = field.find(KEY_VALUE_SEPARATOR);
        if (eqPos == std::string::npos) {
            log("Invalid field format in body");
            return false;
        }

        int tag = std::stoi(field.substr(0, eqPos));
        std::string value = field.substr(eqPos + 1);
        fields[tag] = value;

        pos = nextSep + 1;
    }

    return true;
}

bool ParseFIX::validateChecksum(const std::string& msg, const std::string& checksum) {
    // 计算校验和：所有字符ASCII值的和对256取模
    size_t checksumPos = msg.rfind("10=");
    if (checksumPos == std::string::npos) return false;

    int sum = 0;
    for (size_t i = 0; i < checksumPos; ++i) {
        sum += static_cast<unsigned char>(msg[i]);
    }
    sum = sum % 256;

    // 将计算得到的校验和转换为三位数的字符串
    std::stringstream ss;
    ss << std::setw(3) << std::setfill('0') << sum;
    return ss.str() == checksum;
}

bool ParseFIX::validateRequiredFields(const std::map<int, std::string>& fields) {
    // 验证必填字段
    const std::vector<int> requiredFields = {8, 9, 35, 49, 56, 34, 52}; // BeginString, BodyLength, MsgType, SenderCompID, TargetCompID, MsgSeqNum, SendingTime

    for (int tag : requiredFields) {
        if (fields.find(tag) == fields.end()) {
            std::stringstream ss;
            ss << "Missing required field: " << tag;
            log(ss.str());
            return false;
        }
    }

    return true;
}

void ParseFIX::log(const std::string& msg) {
    if (_logCallback) {
        _logCallback(msg);
    }
}