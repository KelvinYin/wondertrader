/*!
 * \file ParserBA.h
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 *
 * \brief
 */
#pragma once
#include "../Includes/IParserApi.h"
#include "../Share/DLLHelper.hpp"
#include "../API/binance_ws.h"
#include "../WTSTools/WTSLogger.h"

#include <map>

NS_WTP_BEGIN
class WTSTickData;
NS_WTP_END

USING_NS_WTP;

class ParserBA :	public IParserApi
{
public:
	ParserBA();
	virtual ~ParserBA();

//IQuoteParser 接口
public:
	virtual bool init(WTSVariant* config) override;

	virtual void release() override;

    virtual bool connect() { return ba_wsclient_.connect(); }

	virtual bool disconnect() override;

	virtual void subscribe(const CodeSet &vecSymbols) override;

	virtual void registerSpi(IParserSpi* listener) override;

	virtual void handleData(const char* data, int length) override;


private:
	IParserSpi*		m_sink;
	BinanceWSClient ba_wsclient_;

	std::string 	m_strFrontAddr;
    std::string 	m_strStreams;
	IBaseDataMgr*	m_pBaseDataMgr{nullptr};
};
