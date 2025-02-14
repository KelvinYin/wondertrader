#include "WtFixTraderApi.h"
#include "WtFixMsgCodec.h"
#include <sstream>
#include <chrono>

WtFixTraderApi::WtFixTraderApi() : _seqNum(1) {}

WtFixTraderApi::~WtFixTraderApi() {
    disconnect();
}

bool WtFixTraderApi::connect(const std::string& host, int port) {
    _host = host;
    _port = port;
    
    // TODO: 实现实际的TCP连接逻辑
    _connected = true;
    
    if (_logCallback) {
        _logCallback("Connected to FIX server: " + host + ":" + std::to_string(port));
    }
    
    return true;
}

void WtFixTraderApi::disconnect() {
    if (!_connected) return;
    
    // TODO: 实现实际的断开连接逻辑
    _connected = false;
    
    if (_logCallback) {
        _logCallback("Disconnected from FIX server");
    }
}

bool WtFixTraderApi::placeOrder(const std::string& symbol, bool isBuy, double price, int volume) {
    if (!_connected) return false;
    
    std::string orderId = generateOrderId();
    std::map<int, std::string> fields;
    
    // 设置订单字段
    fields[35] = "D";  // MsgType=NewOrderSingle
    fields[11] = orderId;  // ClOrdID
    fields[55] = symbol;  // Symbol
    fields[54] = isBuy ? "1" : "2";  // Side (1=Buy, 2=Sell)
    fields[44] = std::to_string(price);  // Price
    fields[38] = std::to_string(volume);  // OrderQty
    fields[40] = "2";  // OrdType (2=Limit)
    
    std::string msg = WtFixMsgCodec::encode(fields);
    if (sendMessage(msg)) {
        _orderMap[orderId] = symbol;
        return true;
    }
    return false;
}

bool WtFixTraderApi::cancelOrder(const std::string& orderId) {
    if (!_connected) return false;
    
    auto it = _orderMap.find(orderId);
    if (it == _orderMap.end()) return false;
    
    std::map<int, std::string> fields;
    
    // 设置撤单字段
    fields[35] = "F";  // MsgType=OrderCancelRequest
    fields[11] = generateOrderId();  // ClOrdID
    fields[41] = orderId;  // OrigClOrdID
    fields[55] = it->second;  // Symbol
    
    std::string msg = WtFixMsgCodec::encode(fields);
    return sendMessage(msg);
}

bool WtFixTraderApi::queryAccount() {
    if (!_connected) return false;
    
    std::map<int, std::string> fields;
    fields[35] = "AN";  // MsgType=RequestForPositions
    fields[710] = "1";  // PosReqType (1=Positions)
    
    std::string msg = WtFixMsgCodec::encode(fields);
    return sendMessage(msg);
}

bool WtFixTraderApi::queryPositions() {
    if (!_connected) return false;
    
    std::map<int, std::string> fields;
    fields[35] = "AN";  // MsgType=RequestForPositions
    fields[710] = "0";  // PosReqType (0=Positions)
    
    std::string msg = WtFixMsgCodec::encode(fields);
    return sendMessage(msg);
}

bool WtFixTraderApi::queryOrders() {
    if (!_connected) return false;
    
    std::map<int, std::string> fields;
    fields[35] = "AF";  // MsgType=OrderMassStatusRequest
    fields[584] = "1";  // MassStatusReqType (1=Status for all orders)
    
    std::string msg = WtFixMsgCodec::encode(fields);
    return sendMessage(msg);
}

void WtFixTraderApi::handleMessage(const std::string& msg) {
    WtFixApiBase::handleMessage(msg);
}

bool WtFixTraderApi::sendMessage(const std::string& msg) {
    return WtFixApiBase::sendMessage(msg);
}

std::string WtFixTraderApi::generateOrderId() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    
    std::stringstream ss;
    ss << _sender << "_" << timestamp << "_" << _seqNum++;
    return ss.str();
}