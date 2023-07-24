/*!
 * \file ParserBA.cpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 *
 * \brief
 */
#include "ParserBA.h"

#include "../Includes/WTSDataDef.hpp"
#include "../Includes/WTSContractInfo.hpp"
#include "../Includes/WTSVariant.hpp"
#include "../Includes/IBaseDataMgr.h"

#include "../Share/ModuleHelper.hpp"
#include "../Share/TimeUtils.hpp"
#include "../Share/StdUtils.hpp"

#include <boost/filesystem.hpp>

#include <exception>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

namespace rj = rapidjson;


 //By Wesley @ 2022.01.05
#include "../Share/fmtlib.h"
template<typename... Args>
inline void write_log(IParserSpi* sink, WTSLogLevel ll, const char* format, const Args&... args)
{
	if (sink == NULL)
		return;

	static thread_local char buffer[512] = { 0 };
	fmtutil::format_to(buffer, format, args...);

	sink->handleParserLog(ll, buffer);
}

extern "C"
{
	EXPORT_FLAG IParserApi* createParser()
	{
		ParserBA* parser = new ParserBA();
		return parser;
	}

	EXPORT_FLAG void deleteParser(IParserApi* &parser)
	{
		if (NULL != parser)
		{
			delete parser;
			parser = NULL;
		}
	}
};


inline uint32_t strToTime(const char* strTime)
{
	static char str[10] = { 0 };
	const char *pos = strTime;
	int idx = 0;
	auto len = strlen(strTime);
	for(std::size_t i = 0; i < len; i++)
	{
		if(strTime[i] != ':')
		{
			str[idx] = strTime[i];
			idx++;
		}
	}
	str[idx] = '\0';

	return strtoul(str, NULL, 10);
}

inline double checkValid(double val)
{
	if (val == DBL_MAX || val == FLT_MAX)
		return 0;

	return val;
}

ParserBA::ParserBA()
{
}


ParserBA::~ParserBA()
{
}

bool ParserBA::init(WTSVariant* config)
{
	m_strFrontAddr = config->getCString("host");
    m_strStreams   = config->getCString("streams");
    std::string sub_streams = m_strFrontAddr + "stream?streams=";
    sub_streams = sub_streams.append(m_strStreams);

    ba_wsclient_.set_url(sub_streams);
    ba_wsclient_.init();

	return true;
}

void ParserBA::release()
{
	disconnect();
}

bool ParserBA::disconnect()
{
	return true;
}

void ParserBA::handleData(const char* data, int length)
{
	rj::Document document;
    if(document.Parse(data).HasParseError())
	{
        WTSLogger::error("[ParserBA] data format error, not json: %s", data);
		return;
	}

	const rj::Value& data_v = document["data"];
    std::string event_type = data_v["e"].GetString();
    if (event_type == "depthUpdate") // 深度更新消息
    {
    	uint64_t 	event_time = data_v["E"].GetInt64();
        uint64_t    trans_time = data_v["T"].GetInt64();
		std::string symbol     = data_v["s"].GetString();
		uint64_t    first_updateId = data_v["U"].GetInt64();
		uint64_t 	last_updateId  = data_v["u"].GetInt64();
		uint64_t 	prev_last_updateId = data_v["pu"].GetInt64(); // previous u

		WTSTickData* tick = WTSTickData::create(symbol.c_str());
		WTSTickStruct& quote = tick->getTickStruct();
		strcpy(quote.exchg, "Binance");

	    // WTSContractInfo* contract = m_pBaseDataMgr->getContract(symbol.c_str(), quote.exchg);
	    // if (contract == NULL)
	    //     return;
		// tick->setContractInfo(contract);

		rj::SizeType depth_size = 20;
        const rj::Value& depth_b = data_v["b"];
		if (depth_b.Size() > 20)
			depth_size = 20;
		else
			depth_size = depth_b.Size();

        for (rj::SizeType i = 0; i < depth_size; i++)
        {
            const rj::Value& depth = depth_b[i];
            double price = atof(depth[0].GetString());
            double qty = atof(depth[1].GetString());
			quote.bid_prices[i] = checkValid(price);
			quote.bid_qty[i] = qty;
        }

        const rj::Value& depth_a = data_v["a"];
		if (depth_a.Size() > 20)
			depth_size = 20;
		else
			depth_size = depth_a.Size();

        for (rj::SizeType i = 0; i < depth_size; i++)
        {
            const rj::Value& depth = depth_a[i];
            double price = atof(depth[0].GetString());
            double qty = atof(depth[1].GetString());
			quote.ask_prices[i] = checkValid(price);
			quote.ask_qty[i] = checkValid(qty);
        }

		if(m_sink)
			m_sink->handleQuote(tick, 1);

		tick->release();
    }
    else if (event_type == "24hrTicker")
    {

    }
	else
	{
		WTSLogger::debug("[ParserBA] data cannot be parsed, stream: {}", document["stream"].GetString());
	}
}

void ParserBA::subscribe(const CodeSet &vecSymbols)
{
}

void ParserBA::registerSpi(IParserSpi* listener)
{
	m_sink = listener;

	if(m_sink)
	{
		ba_wsclient_.registerSpi(listener);
		m_pBaseDataMgr = m_sink->getBaseDataMgr();
	}
}
