/*
	Date  : 2023/07/15
	C++ library for Binance Future API.
*/
#pragma once
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <exception>

#include <json/json.h>

#define BINANCE_FUTURE_HOST "https://fapi.binance.com"

using namespace std;

class BinaFuturesCPP
{
public:
	void init(string &key, string &secret, string& url);
	void cleanup();

	// Public API
	void get_serverTime(std::string &result);
	void get_exchangeInfo(std::string &result);
	void get_depth(const char *symbol, int limit, std::string &result);
	void get_trades(const char *symbol, int limit, std::string &result);
	void get_aggTrades(const char *symbol, int fromId, time_t startTime, time_t endTime, int limit, std::string &result);
	void get_klines(const char *symbol, const char *interval, int limit, time_t startTime, time_t endTime, std::string &result);
	void get_24hr(const char *symbol, std::string &result);
	void get_price(const char *symbol, std::string &result);
	void get_bookTicker(const char *symbol, std::string &result);

	// API + Secret keys required
	void get_balance(long recvWindow, std::string &result);
	void get_account(long recvWindow, std::string &result);

	void send_order(const char *symbol, const char *side, const char *type, const char *timeInForce, const char *reduceOnly,
					double quantity, double price, const char *newClientOrderId, long recvWindow, std::string &result);

	void send_marketOrder(const char *symbol, const char *side, const char *reduceOnly, double quantity,
						  const char *newClientOrderId, long recvWindow, std::string &result);

	void send_limitOrderBoth() {}
	void send_marketOrderBoth() {}

	void cancel_order(const char *symbol, long orderId, const char *origClientOrderId, long recvWindow, std::string &result);
	void cancel_allOrder(const char *symbol, long recvWindow, std::string &result);

	void get_myTrades(const char *symbol, int limit, long fromId, long recvWindow, std::string &result);

	void get_openOrders(const char *symbol, long recvWindow, std::string &result);
	void get_openOrder(const char *symbol, long orderId, const char *origClientOrderId, long recvWindow, std::string &result);
	void get_allOrders(const char *symbol, long orderId, long startTime, long endTime, int limit, long recvWindow, std::string &result);
	void get_order(const char *symbol, long orderId, const char *origClientOrderId, long recvWindow, std::string &result);

	// API key required
	void start_userDataStream(std::string &result);
	void keep_userDataStream(const char *listenKey);
	void close_userDataStream(const char *listenKey);

private:
	string api_key_{""};
	string secret_key_{""};
	string url_{BINANCE_FUTURE_HOST};
};
