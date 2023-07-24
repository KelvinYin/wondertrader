#include "TraderBA.h"

#include "../Includes/WTSError.hpp"
#include "../Includes/WTSContractInfo.hpp"
#include "../Includes/WTSSessionInfo.hpp"
#include "../Includes/WTSTradeDef.hpp"
#include "../Includes/WTSDataDef.hpp"
#include "../Includes/WTSVariant.hpp"
#include "../Includes/IBaseDataMgr.h"

#include "../Share/decimal.h"

#include <boost/filesystem.hpp>

//By Wesley @ 2022.01.05
#include "../Share/fmtlib.h"
template<typename... Args>
inline void write_log(ITraderSpi* sink, WTSLogLevel ll, const char* format, const Args&... args)
{
	if (sink == NULL)
		return;

	const char* buffer = fmtutil::format(format, args...);

	sink->handleTraderLog(ll, buffer);
}

uint32_t strToTime(const char* strTime)
{
	std::string str;
	const char *pos = strTime;
	while (strlen(pos) > 0)
	{
		if (pos[0] != ':')
		{
			str.append(pos, 1);
		}
		pos++;
	}

	return strtoul(str.c_str(), NULL, 10);
}

extern "C"
{
	EXPORT_FLAG ITraderApi* createTrader()
	{
		TraderBA *instance = new TraderBA();
		return instance;
	}

	EXPORT_FLAG void deleteTrader(ITraderApi* &trader)
	{
		if (NULL != trader)
		{
			delete trader;
			trader = NULL;
		}
	}
}

TraderBA::TraderBA()
	: m_mapPosition(NULL)
	, m_ayOrders(NULL)
	, m_ayTrades(NULL)
	, m_ayPosDetail(NULL)
	, m_uLastQryTime(0)
	, m_iRequestID(0)
	, m_bInQuery(false)
	, m_bStopped(false)
	, m_lastQryTime(0)
{
}


TraderBA::~TraderBA()
{
}

bool TraderBA::init(WTSVariant* params)
{
	m_strApiKey = params->get("apikey")->asCString();
	m_strApiSecret = params->get("apisecret")->asCString();
	m_strPass = params->get("pass")->asCString();

	return true;
}

void TraderBA::release()
{
	m_bStopped = true;

	if (m_ayOrders)
		m_ayOrders->clear();

	if (m_ayPosDetail)
		m_ayPosDetail->clear();

	if (m_mapPosition)
		m_mapPosition->clear();

	if (m_ayTrades)
		m_ayTrades->clear();
}

// void TraderBA::connect()
// {
// 	if (m_pUserAPI)
// 	{
// 		m_pUserAPI->Init();
// 	}

// 	if (m_thrdWorker == NULL)
// 	{
// 		m_thrdWorker.reset(new StdThread([this](){
// 			while (!m_bStopped)
// 			{
// 				if(m_queQuery.empty() || m_bInQuery)
// 				{
// 					std::this_thread::sleep_for(std::chrono::milliseconds(1));
// 					continue;
// 				}

// 				uint64_t curTime = TimeUtils::getLocalTimeNow();
// 				if (curTime - m_lastQryTime < 1000)
// 				{
// 					std::this_thread::sleep_for(std::chrono::milliseconds(50));
// 					continue;
// 				}


// 				m_bInQuery = true;
// 				CommonExecuter& handler = m_queQuery.front();
// 				handler();

// 				{
// 					StdUniqueLock lock(m_mtxQuery);
// 					m_queQuery.pop();
// 				}

// 				m_lastQryTime = TimeUtils::getLocalTimeNow();
// 			}
// 		}));
// 	}
// }

// void TraderBA::disconnect()
// {
// 	m_queQuery.push([this]() {
// 		release();
// 	});

// 	if (m_thrdWorker)
// 	{
// 		m_thrdWorker->join();
// 		m_thrdWorker = NULL;
// 	}
// }

// bool TraderBA::makeEntrustID(char* buffer, int length)
// {
// 	if (buffer == NULL || length == 0)
// 		return false;

// 	try
// 	{
// 		memset(buffer, 0, length);
// 		uint32_t orderref = m_orderRef.fetch_add(1) + 1;
// 		fmt::format_to(buffer, "{:06d}#{:010d}#{:06d}", m_frontID, (uint32_t)m_sessionID, orderref);
// 		return true;
// 	}
// 	catch (...)
// 	{

// 	}

// 	return false;
// }

// void TraderBA::registerSpi(ITraderSpi *listener)
// {
// 	m_sink = listener;
// 	if (m_sink)
// 	{
// 		m_bdMgr = listener->getBaseDataMgr();
// 	}
// }

// uint32_t TraderBA::genRequestID()
// {
// 	return m_iRequestID.fetch_add(1) + 1;
// }

// int TraderBA::orderInsert(WTSEntrust* entrust)
// {
// 	if (m_pUserAPI == NULL || m_wrapperState != WS_ALLREADY)
// 	{
// 		write_log(m_sink, LL_ERROR, "[TraderBA] Trading channel not ready");
// 		return -1;
// 	}

// 	CThostFtdcInputOrderField req;
// 	memset(&req, 0, sizeof(req));
// 	wt_strcpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
// 	wt_strcpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());

// 	wt_strcpy(req.InstrumentID, entrust->getCode());
// 	wt_strcpy(req.ExchangeID, entrust->getExchg());

// 	if (strlen(entrust->getUserTag()) == 0)
// 	{
// 		///报单引用
// 		fmt::format_to(req.OrderRef, "{}", m_orderRef.fetch_add(0));
// 	}
// 	else
// 	{
// 		uint32_t fid, sid, orderref;
// 		extractEntrustID(entrust->getEntrustID(), fid, sid, orderref);
// 		///报单引用
// 		fmt::format_to(req.OrderRef, "{}", orderref);
// 	}

// 	if (strlen(entrust->getUserTag()) > 0)
// 	{
// 		m_eidCache.put(entrust->getEntrustID(), entrust->getUserTag(), 0, [this](const char* message) {
// 			write_log(m_sink, LL_WARN, message);
// 		});
// 	}

// 	///报单价格条件: 限价
// 	req.OrderPriceType = wrapPriceType(entrust->getPriceType(), strcmp(entrust->getExchg(), "CFFEX") == 0);
// 	///买卖方向:
// 	req.Direction = wrapDirectionType(entrust->getDirection(), entrust->getOffsetType());
// 	///组合开平标志: 开仓
// 	req.CombOffsetFlag[0] = wrapOffsetType(entrust->getOffsetType());
// 	///组合投机套保标志
// 	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
// 	///价格
// 	req.LimitPrice = entrust->getPrice();
// 	///数量: 1
// 	req.VolumeTotalOriginal = (int)entrust->getVolume();

// 	if(entrust->getOrderFlag() == WOF_NOR)
// 	{
// 		req.TimeCondition = THOST_FTDC_TC_GFD;
// 		req.VolumeCondition = THOST_FTDC_VC_AV;
// 	}
// 	else if (entrust->getOrderFlag() == WOF_FAK)
// 	{
// 		req.TimeCondition = THOST_FTDC_TC_IOC;
// 		req.VolumeCondition = THOST_FTDC_VC_AV;
// 	}
// 	else if (entrust->getOrderFlag() == WOF_FOK)
// 	{
// 		req.TimeCondition = THOST_FTDC_TC_IOC;
// 		req.VolumeCondition = THOST_FTDC_VC_CV;
// 	}
// 	//req.MinVolume = 1;

// 	///触发条件: 立即
// 	req.ContingentCondition = THOST_FTDC_CC_Immediately;
// 	///强平原因: 非强平
// 	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
// 	///自动挂起标志: 否
// 	req.IsAutoSuspend = 0;
// 	///用户强评标志: 否
// 	req.UserForceClose = 0;

// 	int iResult = m_pUserAPI->ReqOrderInsert(&req, genRequestID());
// 	if (iResult != 0)
// 	{
// 		write_log(m_sink, LL_ERROR, "[TraderBA] Order inserting failed: {}", iResult);
// 	}

// 	return 0;
// }

// int TraderBA::orderAction(WTSEntrustAction* action)
// {
// 	if (m_wrapperState != WS_ALLREADY)
// 		return -1;

// 	uint32_t frontid, sessionid, orderref;
// 	if (!extractEntrustID(action->getEntrustID(), frontid, sessionid, orderref))
// 		return -1;

// 	CThostFtdcInputOrderActionField req;
// 	memset(&req, 0, sizeof(req));
// 	wt_strcpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
// 	wt_strcpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());

// 	///报单引用
// 	fmt::format_to(req.OrderRef, "{}", orderref);
// 	///请求编号
// 	///前置编号
// 	req.FrontID = frontid;
// 	///会话编号
// 	req.SessionID = sessionid;
// 	///操作标志
// 	req.ActionFlag = wrapActionFlag(action->getActionFlag());
// 	///合约代码
// 	wt_strcpy(req.InstrumentID, action->getCode());

// 	req.LimitPrice = action->getPrice();

// 	req.VolumeChange = (int32_t)action->getVolume();

// 	wt_strcpy(req.OrderSysID, action->getOrderID());
// 	wt_strcpy(req.ExchangeID, action->getExchg());

// 	int iResult = m_pUserAPI->ReqOrderAction(&req, genRequestID());
// 	if (iResult != 0)
// 	{
// 		write_log(m_sink, LL_ERROR, "[TraderBA] Sending cancel request failed: {}", iResult);
// 	}

// 	return 0;
// }

// int TraderBA::queryAccount()
// {
// 	if (m_pUserAPI == NULL || m_wrapperState != WS_ALLREADY)
// 	{
// 		return -1;
// 	}

// 	{
// 		StdUniqueLock lock(m_mtxQuery);
// 		m_queQuery.push([this]() {
// 			CThostFtdcQryTradingAccountField req;
// 			memset(&req, 0, sizeof(req));
// 			wt_strcpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
// 			wt_strcpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());
// 			m_pUserAPI->ReqQryTradingAccount(&req, genRequestID());
// 		});
// 	}

// 	//triggerQuery();

// 	return 0;
// }

// int TraderBA::queryPositions()
// {
// 	if (m_pUserAPI == NULL || m_wrapperState != WS_ALLREADY)
// 	{
// 		return -1;
// 	}

// 	{
// 		StdUniqueLock lock(m_mtxQuery);
// 		m_queQuery.push([this]() {
// 			CThostFtdcQryInvestorPositionField req;
// 			memset(&req, 0, sizeof(req));
// 			wt_strcpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
// 			wt_strcpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());
// 			m_pUserAPI->ReqQryInvestorPosition(&req, genRequestID());
// 		});
// 	}

// 	//triggerQuery();

// 	return 0;
// }

// int TraderBA::queryOrders()
// {
// 	if (m_pUserAPI == NULL || m_wrapperState != WS_ALLREADY)
// 	{
// 		return -1;
// 	}

// 	{
// 		StdUniqueLock lock(m_mtxQuery);
// 		m_queQuery.push([this]() {
// 			CThostFtdcQryOrderField req;
// 			memset(&req, 0, sizeof(req));
// 			wt_strcpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
// 			wt_strcpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());

// 			m_pUserAPI->ReqQryOrder(&req, genRequestID());
// 		});

// 		//triggerQuery();
// 	}

// 	return 0;
// }

// int TraderBA::queryTrades()
// {
// 	if (m_pUserAPI == NULL || m_wrapperState != WS_ALLREADY)
// 	{
// 		return -1;
// 	}

// 	{
// 		StdUniqueLock lock(m_mtxQuery);
// 		m_queQuery.push([this]() {
// 			CThostFtdcQryTradeField req;
// 			memset(&req, 0, sizeof(req));
// 			wt_strcpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
// 			wt_strcpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());

// 			m_pUserAPI->ReqQryTrade(&req, genRequestID());
// 		});
// 	}

// 	//triggerQuery();

// 	return 0;
// }

// int TraderBA::querySettlement(uint32_t uDate)
// {
// 	if (m_pUserAPI == NULL || m_wrapperState != WS_ALLREADY)
// 	{
// 		return -1;
// 	}

// 	m_strSettleInfo.clear();
// 	StdUniqueLock lock(m_mtxQuery);
// 	m_queQuery.push([this, uDate]() {
// 		CThostFtdcQrySettlementInfoField req;
// 		memset(&req, 0, sizeof(req));
// 		wt_strcpy(req.BrokerID, m_strBroker.c_str(), m_strBroker.size());
// 		wt_strcpy(req.InvestorID, m_strUser.c_str(), m_strUser.size());
// 		fmt::format_to(req.TradingDay, "{}", uDate);

// 		m_pUserAPI->ReqQrySettlementInfo(&req, genRequestID());
// 	});

// 	//triggerQuery();

// 	return 0;
// }

// void TraderBA::OnRspQryOrder(WTSOrderInfo *pOrder)
// {
// 	m_bInQuery = false;

// 	if (!IsErrorRspInfo(pRspInfo) && pOrder)
// 	{
// 		if (NULL == m_ayOrders)
// 			m_ayOrders = WTSArray::create();

// 		WTSOrderInfo* orderInfo = makeOrderInfo(pOrder);
// 		if (orderInfo)
// 		{
// 			m_ayOrders->append(orderInfo, false);
// 		}
// 	}

// 	if (m_sink)
// 		m_sink->onRspOrders(m_ayOrders);

// 	if (m_ayOrders)
// 		m_ayOrders->clear();
// }

// void TraderBA::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
// {
// 	m_sink->handleTraderLog(LL_ERROR, fmtutil::format("{} rsp error: {} : {}", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg));
// }

// void TraderBA::OnRtnOrder(CThostFtdcOrderField *pOrder)
// {
// 	WTSOrderInfo *orderInfo = makeOrderInfo(pOrder);
// 	if (orderInfo)
// 	{
// 		if (m_sink)
// 			m_sink->onPushOrder(orderInfo);

// 		orderInfo->release();
// 	}

// 	//ReqQryTradingAccount();
// }

// void TraderBA::OnRtnTrade(CThostFtdcTradeField *pTrade)
// {
// 	WTSTradeInfo *tRecord = makeTradeRecord(pTrade);
// 	if (tRecord)
// 	{
// 		if (m_sink)
// 			m_sink->onPushTrade(tRecord);

// 		tRecord->release();
// 	}
// }

// int TraderBA::wrapDirectionType(WTSDirectionType dirType, WTSOffsetType offsetType)
// {
// 	if (WDT_LONG == dirType)
// 		if (offsetType == WOT_OPEN)
// 			return THOST_FTDC_D_Buy;
// 		else
// 			return THOST_FTDC_D_Sell;
// 	else
// 		if (offsetType == WOT_OPEN)
// 			return THOST_FTDC_D_Sell;
// 		else
// 			return THOST_FTDC_D_Buy;
// }

// WTSDirectionType TraderBA::wrapDirectionType(TThostFtdcDirectionType dirType, TThostFtdcOffsetFlagType offsetType)
// {
// 	if (THOST_FTDC_D_Buy == dirType)
// 		if (offsetType == THOST_FTDC_OF_Open)
// 			return WDT_LONG;
// 		else
// 			return WDT_SHORT;
// 	else
// 		if (offsetType == THOST_FTDC_OF_Open)
// 			return WDT_SHORT;
// 		else
// 			return WDT_LONG;
// }

// WTSDirectionType TraderBA::wrapPosDirection(TThostFtdcPosiDirectionType dirType)
// {
// 	if (THOST_FTDC_PD_Long == dirType)
// 		return WDT_LONG;
// 	else
// 		return WDT_SHORT;
// }

// int TraderBA::wrapOffsetType(WTSOffsetType offType)
// {
// 	if (WOT_OPEN == offType)
// 		return THOST_FTDC_OF_Open;
// 	else if (WOT_CLOSE == offType)
// 		return THOST_FTDC_OF_Close;
// 	else if (WOT_CLOSETODAY == offType)
// 		return THOST_FTDC_OF_CloseToday;
// 	else if (WOT_CLOSEYESTERDAY == offType)
// 		return THOST_FTDC_OF_Close;
// 	else
// 		return THOST_FTDC_OF_ForceClose;
// }

// WTSOffsetType TraderBA::wrapOffsetType(TThostFtdcOffsetFlagType offType)
// {
// 	if (THOST_FTDC_OF_Open == offType)
// 		return WOT_OPEN;
// 	else if (THOST_FTDC_OF_Close == offType)
// 		return WOT_CLOSE;
// 	else if (THOST_FTDC_OF_CloseToday == offType)
// 		return WOT_CLOSETODAY;
// 	else
// 		return WOT_FORCECLOSE;
// }

// int TraderBA::wrapPriceType(WTSPriceType priceType, bool isCFFEX /* = false */)
// {
// 	if (WPT_ANYPRICE == priceType)
// 		return isCFFEX ? THOST_FTDC_OPT_FiveLevelPrice : THOST_FTDC_OPT_AnyPrice;
// 	else if (WPT_LIMITPRICE == priceType)
// 		return THOST_FTDC_OPT_LimitPrice;
// 	else if (WPT_BESTPRICE == priceType)
// 		return THOST_FTDC_OPT_BestPrice;
// 	else
// 		return THOST_FTDC_OPT_LastPrice;
// }

// WTSPriceType TraderBA::wrapPriceType(TThostFtdcOrderPriceTypeType priceType)
// {
// 	if (THOST_FTDC_OPT_AnyPrice == priceType || THOST_FTDC_OPT_FiveLevelPrice == priceType)
// 		return WPT_ANYPRICE;
// 	else if (THOST_FTDC_OPT_LimitPrice == priceType)
// 		return WPT_LIMITPRICE;
// 	else if (THOST_FTDC_OPT_BestPrice == priceType)
// 		return WPT_BESTPRICE;
// 	else
// 		return WPT_LASTPRICE;
// }

// int TraderBA::wrapTimeCondition(WTSTimeCondition timeCond)
// {
// 	if (WTC_IOC == timeCond)
// 		return THOST_FTDC_TC_IOC;
// 	else if (WTC_GFD == timeCond)
// 		return THOST_FTDC_TC_GFD;
// 	else
// 		return THOST_FTDC_TC_GFS;
// }

// WTSTimeCondition TraderBA::wrapTimeCondition(TThostFtdcTimeConditionType timeCond)
// {
// 	if (THOST_FTDC_TC_IOC == timeCond)
// 		return WTC_IOC;
// 	else if (THOST_FTDC_TC_GFD == timeCond)
// 		return WTC_GFD;
// 	else
// 		return WTC_GFS;
// }

// WTSOrderState TraderBA::wrapOrderState(TThostFtdcOrderStatusType orderState)
// {
// 	if (orderState != THOST_FTDC_OST_Unknown)
// 		return (WTSOrderState)orderState;
// 	else
// 		return WOS_Submitting;
// }

// int TraderBA::wrapActionFlag(WTSActionFlag actionFlag)
// {
// 	if (WAF_CANCEL == actionFlag)
// 		return THOST_FTDC_AF_Delete;
// 	else
// 		return THOST_FTDC_AF_Modify;
// }


// WTSOrderInfo* TraderBA::makeOrderInfo(CThostFtdcOrderField* orderField)
// {
// 	WTSContractInfo* contract = m_bdMgr->getContract(orderField->InstrumentID, orderField->ExchangeID);
// 	if (contract == NULL)
// 		return NULL;

// 	WTSOrderInfo* pRet = WTSOrderInfo::create();
// 	pRet->setContractInfo(contract);
// 	pRet->setPrice(orderField->LimitPrice);
// 	pRet->setVolume(orderField->VolumeTotalOriginal);
// 	pRet->setDirection(wrapDirectionType(orderField->Direction, orderField->CombOffsetFlag[0]));
// 	pRet->setPriceType(wrapPriceType(orderField->OrderPriceType));
// 	pRet->setOffsetType(wrapOffsetType(orderField->CombOffsetFlag[0]));

// 	if (orderField->TimeCondition == THOST_FTDC_TC_GFD)
// 	{
// 		pRet->setOrderFlag(WOF_NOR);
// 	}
// 	else if (orderField->TimeCondition == THOST_FTDC_TC_IOC)
// 	{
// 		if (orderField->VolumeCondition == THOST_FTDC_VC_AV || orderField->VolumeCondition == THOST_FTDC_VC_MV)
// 			pRet->setOrderFlag(WOF_FAK);
// 		else
// 			pRet->setOrderFlag(WOF_FOK);
// 	}

// 	pRet->setVolTraded(orderField->VolumeTraded);
// 	pRet->setVolLeft(orderField->VolumeTotal);

// 	pRet->setCode(orderField->InstrumentID);
// 	pRet->setExchange(contract->getExchg());

// 	uint32_t uDate = strtoul(orderField->InsertDate, NULL, 10);
// 	std::string strTime = orderField->InsertTime;
// 	StrUtil::replace(strTime, ":", "");
// 	uint32_t uTime = strtoul(strTime.c_str(), NULL, 10);
// 	if (uTime >= 210000 && uDate == m_lDate)
// 	{
// 		uDate = TimeUtils::getNextDate(uDate, -1);
// 	}

// 	pRet->setOrderDate(uDate);
// 	pRet->setOrderTime(TimeUtils::makeTime(uDate, uTime * 1000));

// 	pRet->setOrderState(wrapOrderState(orderField->OrderStatus));
// 	if (orderField->OrderSubmitStatus >= THOST_FTDC_OSS_InsertRejected)
// 		pRet->setError(true);

// 	generateEntrustID(pRet->getEntrustID(), orderField->FrontID, orderField->SessionID, atoi(orderField->OrderRef));
// 	pRet->setOrderID(orderField->OrderSysID);

// 	pRet->setStateMsg(orderField->StatusMsg);


// 	const char* usertag = m_eidCache.get(pRet->getEntrustID());
// 	if(strlen(usertag) == 0)
// 	{
// 		pRet->setUserTag(pRet->getEntrustID());
// 	}
// 	else
// 	{
// 		pRet->setUserTag(usertag);

// 		if (strlen(pRet->getOrderID()) > 0)
// 		{
// 			m_oidCache.put(StrUtil::trim(pRet->getOrderID()).c_str(), usertag, 0, [this](const char* message) {
// 				write_log(m_sink, LL_ERROR, message);
// 			});
// 		}
// 	}

// 	return pRet;
// }

// WTSEntrust* TraderBA::makeEntrust(CThostFtdcInputOrderField *entrustField)
// {
// 	WTSContractInfo* ct = m_bdMgr->getContract(entrustField->InstrumentID, entrustField->ExchangeID);
// 	if (ct == NULL)
// 		return NULL;

// 	WTSEntrust* pRet = WTSEntrust::create(
// 		entrustField->InstrumentID,
// 		entrustField->VolumeTotalOriginal,
// 		entrustField->LimitPrice,
// 		ct->getExchg());

// 	pRet->setContractInfo(ct);

// 	pRet->setDirection(wrapDirectionType(entrustField->Direction, entrustField->CombOffsetFlag[0]));
// 	pRet->setPriceType(wrapPriceType(entrustField->OrderPriceType));
// 	pRet->setOffsetType(wrapOffsetType(entrustField->CombOffsetFlag[0]));

// 	if (entrustField->TimeCondition == THOST_FTDC_TC_GFD)
// 	{
// 		pRet->setOrderFlag(WOF_NOR);
// 	}
// 	else if (entrustField->TimeCondition == THOST_FTDC_TC_IOC)
// 	{
// 		if (entrustField->VolumeCondition == THOST_FTDC_VC_AV || entrustField->VolumeCondition == THOST_FTDC_VC_MV)
// 			pRet->setOrderFlag(WOF_FAK);
// 		else
// 			pRet->setOrderFlag(WOF_FOK);
// 	}

// 	//pRet->setEntrustID(generateEntrustID(m_frontID, m_sessionID, atoi(entrustField->OrderRef)).c_str());
// 	generateEntrustID(pRet->getEntrustID(), m_frontID, m_sessionID, atoi(entrustField->OrderRef));

// 	const char* usertag = m_eidCache.get(pRet->getEntrustID());
// 	if (strlen(usertag) > 0)
// 		pRet->setUserTag(usertag);

// 	return pRet;
// }

// WTSError* TraderBA::makeError(CThostFtdcRspInfoField* rspInfo, WTSErroCode ec /* = WEC_NONE */)
// {
// 	WTSError* pRet = WTSError::create(ec, rspInfo->ErrorMsg);
// 	return pRet;
// }

// WTSTradeInfo* TraderBA::makeTradeRecord(CThostFtdcTradeField *tradeField)
// {
// 	WTSContractInfo* contract = m_bdMgr->getContract(tradeField->InstrumentID, tradeField->ExchangeID);
// 	if (contract == NULL)
// 		return NULL;

// 	WTSTradeInfo *pRet = WTSTradeInfo::create(tradeField->InstrumentID, contract->getExchg());
// 	pRet->setVolume(tradeField->Volume);
// 	pRet->setPrice(tradeField->Price);
// 	pRet->setTradeID(tradeField->TradeID);
// 	pRet->setContractInfo(contract);

// 	std::string strTime = tradeField->TradeTime;
// 	StrUtil::replace(strTime, ":", "");
// 	uint32_t uTime = strtoul(strTime.c_str(), NULL, 10);
// 	uint32_t uDate = strtoul(tradeField->TradeDate, NULL, 10);

// 	//如果是夜盘时间，并且成交日期等于交易日，说明成交日期是需要修正
// 	//因为夜盘是提前的，成交日期必然小于交易日
// 	//但是这里只能做一个简单修正了
// 	if(uTime >= 210000 && uDate == m_lDate)
// 	{
// 		uDate = TimeUtils::getNextDate(uDate, -1);
// 	}

// 	pRet->setTradeDate(uDate);
// 	pRet->setTradeTime(TimeUtils::makeTime(uDate, uTime * 1000));

// 	WTSDirectionType dType = wrapDirectionType(tradeField->Direction, tradeField->OffsetFlag);

// 	pRet->setDirection(dType);
// 	pRet->setOffsetType(wrapOffsetType(tradeField->OffsetFlag));
// 	pRet->setRefOrder(tradeField->OrderSysID);
// 	pRet->setTradeType((WTSTradeType)tradeField->TradeType);

// 	double amount = contract->getCommInfo()->getVolScale()*tradeField->Volume*pRet->getPrice();
// 	pRet->setAmount(amount);

// 	const char* usertag = m_oidCache.get(StrUtil::trim(pRet->getRefOrder()).c_str());
// 	if (strlen(usertag))
// 		pRet->setUserTag(usertag);

// 	return pRet;
// }

// void TraderBA::generateEntrustID(char* buffer, uint32_t frontid, uint32_t sessionid, uint32_t orderRef)
// {
// 	fmtutil::format_to(buffer, "{:06d}#{:010d}#{:06d}", frontid, sessionid, orderRef);
// }

// bool TraderBA::extractEntrustID(const char* entrustid, uint32_t &frontid, uint32_t &sessionid, uint32_t &orderRef)
// {
// 	thread_local static char buffer[64];
// 	wt_strcpy(buffer, entrustid);
// 	char* s = buffer;
// 	auto idx = StrUtil::findFirst(s, '#');
// 	if (idx == std::string::npos)
// 		return false;
// 	s[idx] = '\0';
// 	frontid = strtoul(s, NULL, 10);
// 	s += idx + 1;

// 	idx = StrUtil::findFirst(s, '#');
// 	if (idx == std::string::npos)
// 		return false;
// 	s[idx] = '\0';
// 	sessionid = strtoul(s, NULL, 10);
// 	s += idx + 1;

// 	orderRef = strtoul(s, NULL, 10);

// 	return true;
// }

// bool TraderBA::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
// {
// 	if (pRspInfo && pRspInfo->ErrorID != 0)
// 		return true;

// 	return false;
// }

// void TraderBA::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
// {
// 	WTSEntrust* entrust = makeEntrust(pInputOrder);
// 	if (entrust)
// 	{
// 		WTSError *err = makeError(pRspInfo, WEC_ORDERINSERT);
// 		//g_orderMgr.onRspEntrust(entrust, err);
// 		if (m_sink)
// 			m_sink->onRspEntrust(entrust, err);
// 		entrust->release();
// 		err->release();
// 	}
// }
