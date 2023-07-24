/*!
 * \file WTSBaseDataMgr.cpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 *
 * \brief
 */
#include "WTSBaseDataMgr.h"
#include "../WTSUtils/WTSCfgLoader.h"
#include "WTSLogger.h"

#include "../Includes/WTSContractInfo.hpp"
#include "../Includes/WTSSessionInfo.hpp"
#include "../Includes/WTSVariant.hpp"

#include "../Share/StrUtil.hpp"
#include "../Share/StdUtils.hpp"

const char *DEFAULT_HOLIDAY_TPL = "CHINA";

WTSBaseDataMgr::WTSBaseDataMgr()
	: m_mapExchgContract(NULL), m_mapContracts(NULL)
{
	m_mapExchgContract = WTSExchgContract::create();
	m_mapContracts = WTSContractMap::create();
}

WTSBaseDataMgr::~WTSBaseDataMgr()
{
	if (m_mapExchgContract)
	{
		m_mapExchgContract->release();
		m_mapExchgContract = NULL;
	}

	if (m_mapContracts)
	{
		m_mapContracts->release();
		m_mapContracts = NULL;
	}
}

WTSContractInfo *WTSBaseDataMgr::getContract(const char *code, const char *exchg)
{
	if (code == nullptr)
		return NULL;

	auto lKey = LongKey(code);

	if (strlen(exchg) == 0)
	{
		auto it = m_mapContracts->find(lKey);
		if (it == m_mapContracts->end())
			return NULL;

		WTSArray *ayInst = (WTSArray *)it->second;
		if (ayInst == NULL || ayInst->size() == 0)
			return NULL;

		return (WTSContractInfo *)ayInst->at(0);
	}
	else
	{
		auto sKey = ShortKey(exchg);
		auto it = m_mapExchgContract->find(sKey);
		if (it != m_mapExchgContract->end())
		{
			WTSContractList *contractList = (WTSContractList *)it->second;
			auto it = contractList->find(lKey);
			if (it != contractList->end())
			{
				return (WTSContractInfo *)it->second;
			}
		}
	}

	return NULL;
}

WTSArray *WTSBaseDataMgr::getContracts(const char *exchg /* = "" */)
{
	WTSArray *ay = WTSArray::create();
	if (strlen(exchg) > 0)
	{
		auto it = m_mapExchgContract->find(ShortKey(exchg));
		if (it != m_mapExchgContract->end())
		{
			WTSContractList *contractList = (WTSContractList *)it->second;
			auto it2 = contractList->begin();
			for (; it2 != contractList->end(); it2++)
			{
				ay->append(it2->second, true);
			}
		}
	}
	else
	{
		auto it = m_mapExchgContract->begin();
		for (; it != m_mapExchgContract->end(); it++)
		{
			WTSContractList *contractList = (WTSContractList *)it->second;
			auto it2 = contractList->begin();
			for (; it2 != contractList->end(); it2++)
			{
				ay->append(it2->second, true);
			}
		}
	}

	return ay;
}

void WTSBaseDataMgr::release()
{
	if (m_mapExchgContract)
	{
		m_mapExchgContract->release();
		m_mapExchgContract = NULL;
	}
}

bool WTSBaseDataMgr::loadContracts(uint32_t type, const char *filename)
{
	WTSVariant *root = nullptr;
	if (type == CONTRACT_TYPE_FILE)
	{
		if (!StdFile::exists(filename))
		{
			WTSLogger::error("[WTSBaseDataMgr::loadContracts] Contracts configuration file {} not exists", filename);
			return false;
		}
		root = WTSCfgLoader::load_from_file(filename);
		if (root == nullptr)
		{
			WTSLogger::error("[WTSBaseDataMgr::loadContracts] Loading contracts config file {} failed", filename);
			return false;
		}
	}
	else if (type == CONTRACT_TYPE_URL)
	{
		// TODO
	}
	else
	{
		WTSLogger::error("[WTSBaseDataMgr::loadContracts] Loading contracts config type {} failed", type);
		return false;
	}

	WTSVariant *symbols = root->get("symbols");
	if (symbols && symbols->type() == WTSVariant::VT_Array)
	{
		for (uint32_t idx = 0; idx < symbols->size(); idx++)
		{
			WTSVariant* jSymbol = symbols->get(idx);
			if (jSymbol && jSymbol->type() == WTSVariant::VT_Object)
			{
				std::string symbol = jSymbol->getCString("symbol");
				std::string exchg = "BINANCE";
				WTSContractInfo *cInfo = WTSContractInfo::create(jSymbol->getCString("symbol"),
																 "BINANCE",
																 jSymbol->getCString("baseAsset"),
																 jSymbol->getCString("quoteAsset"),
																 jSymbol->getCString("marginAsset"));
				double maxMktQty = 1000000;
				double maxLmtQty = 1000000;
				double minMktQty = 1;
				double minLmtQty = 1;
				double tick = 0.0;
				double maxPrice = 0.0;
				double minPrice = 0.0;
				double lotSize = 0.0;
				double minNotional = 0.0;
				double multiplierDown = 0.0;
				double multiplierUp = 0.0;
				uint32_t ordLmtNum = 200;
				uint32_t ordAlgoNum = 10;

				WTSVariant* filters = jSymbol->get("filters");
				if (filters && filters->type() == WTSVariant::VT_Array)
				{
					for (uint32_t i = 0; i < filters->size(); i++)
					{
						WTSVariant* filter = filters->get(i);
						if (filter && filter->type() == WTSVariant::VT_Object)
						{
							std::string type = filter->getCString("filterType");
							if (type == "PRICE_FILTER")
							{
								minPrice = filter->getDouble("minPrice");
								maxPrice = filter->getDouble("maxPrice");
								tick = filter->getDouble("tickSize");
							}
							else if (type == "LOT_SIZE")
							{
								minLmtQty = filter->getDouble("minQty");
								maxLmtQty = filter->getDouble("maxQty");
								lotSize = minLmtQty;
								// stepSize = filter->getDouble("stepSize");
							}
							else if (type == "MARKET_LOT_SIZE")
							{
								minMktQty = filter->getDouble("minQty");
								maxMktQty = filter->getDouble("maxQty");
							}
							else if (type == "MIN_NOTIONAL")
							{
								minNotional = filter->getDouble("notional");
							}
							else if (type == "PERCENT_PRICE")
							{
								multiplierDown = filter->getDouble("multiplierDown");
								multiplierUp = filter->getDouble("multiplierUp");
							}
							else if (type == "MAX_NUM_ORDERS")
							{
								ordLmtNum = filter->getUInt32("limit");
							}
							else if (type == "MAX_NUM_ALGO_ORDERS")
							{
								ordAlgoNum = filter->getUInt32("limit");
							}
						}
					}
				}

				cInfo->setVolumeLimits(maxMktQty, maxLmtQty, minMktQty, minLmtQty);
				cInfo->setPriceBounds(minPrice, maxPrice, tick);
				cInfo->setLotBounds(lotSize, lotSize, minNotional);
				cInfo->setPercentPrice(multiplierDown, multiplierUp);

				uint64_t opendate = 0;
				uint64_t expiredate = 0;
				if (jSymbol->has("onboardDate"))
					opendate = jSymbol->getUInt64("onboardDate");
				if (jSymbol->has("deliveryDate"))
					expiredate = jSymbol->getUInt64("deliveryDate");
				cInfo->setDates(opendate, expiredate);

				double maintMargin = 0;
				double requiredMargin = 0;
				if (jSymbol->has("maintMarginPercent"))
					maintMargin = jSymbol->getDouble("maintMarginPercent");
				if (jSymbol->has("requiredMarginPercent"))
					requiredMargin = jSymbol->getDouble("requiredMarginPercent");
				cInfo->setMarginRatios(maintMargin, requiredMargin);

				WTSContractList *contractList = (WTSContractList *)m_mapExchgContract->get(ShortKey(cInfo->getExchg()));
				if (contractList == NULL)
				{
					contractList = WTSContractList::create();
					m_mapExchgContract->add(ShortKey(cInfo->getExchg()), contractList, false);
				}
				contractList->add(LongKey(cInfo->getSymbol()), cInfo, false);

				LongKey key = LongKey(cInfo->getSymbol());
				WTSArray *ayInst = (WTSArray *)m_mapContracts->get(key);
				if (ayInst == NULL)
				{
					ayInst = WTSArray::create();
					m_mapContracts->add(key, ayInst, false);
				}

				ayInst->append(cInfo, true);
			}
		}
	}

	WTSLogger::info("Contracts configuration file {} loaded, {} exchanges", filename, m_mapExchgContract->size());
	root->release();
	return true;
}
