/*!
 * \file TraderBA.h
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 *
 * \brief
 */
#pragma once

#include <string>
#include <queue>
#include <stdint.h>

#include "../Includes/WTSTypes.h"
#include "../Includes/ITraderApi.h"
#include "../Includes/WTSCollection.hpp"

#include "../Share/StdUtils.hpp"
#include "../Share/DLLHelper.hpp"
#include "../Share/WtKVCache.hpp"

USING_NS_WTP;

class TraderBA  : public ITraderApi
{
public:
	TraderBA();
	virtual ~TraderBA();

	//////////////////////////////////////////////////////////////////////////
	//ITraderApi接口
public:
	virtual bool init(WTSVariant* params) override;

	virtual void release() override;

// 	virtual void registerSpi(ITraderSpi *listener) override;

// 	virtual bool makeEntrustID(char* buffer, int length) override;

// 	virtual void connect() override;

// 	virtual void disconnect() override;

// 	virtual bool isConnected() override;

// 	virtual int login(const char* user, const char* pass, const char* productInfo) override;

// 	virtual int logout() override;

// 	virtual int orderInsert(WTSEntrust* eutrust) override;

// 	virtual int orderAction(WTSEntrustAction* action) override;

// 	virtual int queryAccount() override;

// 	virtual int queryPositions() override;

// 	virtual int queryOrders() override;

// 	virtual int queryTrades() override;

// 	virtual int querySettlement(uint32_t uDate) override;

// 	//////////////////////////////////////////////////////////////////////////
// 	//CTP交易接口实现
// public:
// 	///请求查询成交响应
// 	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

// 	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

// 	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

// 	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder) override;

// 	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade) override;

// 	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) override;

// 	virtual void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus) override;

// private:
// 	/*
// 	*	检查错误信息
// 	*/
// 	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

// 	int wrapPriceType(WTSPriceType priceType, bool isCFFEX = false);
// 	int wrapDirectionType(WTSDirectionType dirType, WTSOffsetType offType);
// 	int wrapOffsetType(WTSOffsetType offType);
// 	int	wrapTimeCondition(WTSTimeCondition timeCond);
// 	int wrapActionFlag(WTSActionFlag actionFlag);

// 	WTSPriceType		wrapPriceType(TThostFtdcOrderPriceTypeType priceType);
// 	WTSDirectionType	wrapDirectionType(TThostFtdcDirectionType dirType, TThostFtdcOffsetFlagType offType);
// 	WTSDirectionType	wrapPosDirection(TThostFtdcPosiDirectionType dirType);
// 	WTSOffsetType		wrapOffsetType(TThostFtdcOffsetFlagType offType);
// 	WTSTimeCondition	wrapTimeCondition(TThostFtdcTimeConditionType timeCond);
// 	WTSOrderState		wrapOrderState(TThostFtdcOrderStatusType orderState);

// 	WTSOrderInfo*	makeOrderInfo(CThostFtdcOrderField* orderField);
// 	WTSEntrust*		makeEntrust(CThostFtdcInputOrderField *entrustField);
// 	WTSError*		makeError(CThostFtdcRspInfoField* rspInfo, WTSErroCode ec = WEC_NONE);
// 	WTSTradeInfo*	makeTradeRecord(CThostFtdcTradeField *tradeField);

// 	void			generateEntrustID(char* buffer, uint32_t frontid, uint32_t sessionid, uint32_t orderRef);
// 	bool			extractEntrustID(const char* entrustid, uint32_t &frontid, uint32_t &sessionid, uint32_t &orderRef);

// 	uint32_t		genRequestID();

protected:
	std::string		m_strApiKey;
	std::string		m_strApiSecret;
	std::string		m_strPass;

	ITraderSpi*		m_sink;
	uint64_t		m_uLastQryTime;

	uint32_t					m_lDate;
	std::atomic<uint32_t>		m_orderRef;		//报单引用

	std::atomic<uint32_t>		m_iRequestID;

	typedef WTSHashMap<std::string> PositionMap;
	PositionMap*				m_mapPosition;
	WTSArray*					m_ayTrades;
	WTSArray*					m_ayOrders;
	WTSArray*					m_ayPosDetail;

	IBaseDataMgr*				m_bdMgr;

	typedef std::queue<CommonExecuter>	QueryQue;
	QueryQue				m_queQuery;
	bool					m_bInQuery;
	StdUniqueMutex			m_mtxQuery;
	uint64_t				m_lastQryTime;

	bool					m_bStopped;
	StdThreadPtr			m_thrdWorker;

	//委托单标记缓存器
	WtKVCache		m_eidCache;
	//订单标记缓存器
	WtKVCache		m_oidCache;
};
