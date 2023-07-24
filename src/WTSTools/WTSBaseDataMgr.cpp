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

const char* DEFAULT_HOLIDAY_TPL = "CHINA";

WTSBaseDataMgr::WTSBaseDataMgr()
	: m_mapExchgContract(NULL)
	, m_mapContracts(NULL)
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

	if(m_mapContracts)
	{
		m_mapContracts->release();
		m_mapContracts = NULL;
	}
}

WTSContractInfo* WTSBaseDataMgr::getContract(const char* code, const char* exchg)
{
	if(code == nullptr)
		return NULL;

	auto lKey = LongKey(code);

	if (strlen(exchg) == 0)
	{
		auto it = m_mapContracts->find(lKey);
		if (it == m_mapContracts->end())
			return NULL;

		WTSArray* ayInst = (WTSArray*)it->second;
		if (ayInst == NULL || ayInst->size() == 0)
			return NULL;

		return (WTSContractInfo*)ayInst->at(0);
	}
	else
	{
		auto sKey = ShortKey(exchg);
		auto it = m_mapExchgContract->find(sKey);
		if (it != m_mapExchgContract->end())
		{
			WTSContractList* contractList = (WTSContractList*)it->second;
			auto it = contractList->find(lKey);
			if (it != contractList->end())
			{
				return (WTSContractInfo*)it->second;
			}
		}

	}

	return NULL;
}

WTSArray* WTSBaseDataMgr::getContracts(const char* exchg /* = "" */)
{
	WTSArray* ay = WTSArray::create();
	if(strlen(exchg) > 0)
	{
		auto it = m_mapExchgContract->find(ShortKey(exchg));
		if (it != m_mapExchgContract->end())
		{
			WTSContractList* contractList = (WTSContractList*)it->second;
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
			WTSContractList* contractList = (WTSContractList*)it->second;
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

bool WTSBaseDataMgr::loadContracts(const char* http_url)
{
	if (!StdFile::exists(http_url))
	{
		WTSLogger::error("Contracts configuration file {} not exists", http_url);
		return false;
	}

	WTSVariant* root = WTSCfgLoader::load_from_file(http_url);
	if (root == NULL)
	{
		WTSLogger::error("Loading contracts config file {} failed", http_url);
		return false;
	}

	for(const std::string& exchg : root->memberNames())
	{
		WTSVariant* jExchg = root->get(exchg);

		for(const std::string& code : jExchg->memberNames())
		{
			WTSVariant* jcInfo = jExchg->get(code);

			WTSContractInfo* cInfo = WTSContractInfo::create(code.c_str(),
				jcInfo->getCString("exchg"),
				jcInfo->getCString("base"),
				jcInfo->getCString("quote"),
				jcInfo->getCString("margin"));


			uint32_t maxMktQty = 1000000;
			uint32_t maxLmtQty = 1000000;
			uint32_t minMktQty = 1;
			uint32_t minLmtQty = 1;
			if (jcInfo->has("maxmarketqty"))
				maxMktQty = jcInfo->getUInt32("maxmarketqty");
			if (jcInfo->has("maxlimitqty"))
				maxLmtQty = jcInfo->getUInt32("maxlimitqty");
			if (jcInfo->has("minmarketqty"))
				minMktQty = jcInfo->getUInt32("minmarketqty");
			if (jcInfo->has("minlimitqty"))
				minLmtQty = jcInfo->getUInt32("minlimitqty");
			cInfo->setVolumeLimits(maxMktQty, maxLmtQty, minMktQty, minLmtQty);

			uint32_t opendate = 0;
			uint32_t expiredate = 0;
			if (jcInfo->has("opendate"))
				opendate = jcInfo->getUInt32("opendate");
			if (jcInfo->has("expiredate"))
				expiredate = jcInfo->getUInt32("expiredate");
			cInfo->setDates(opendate, expiredate);

			double lMargin = 0;
			double sMargin = 0;
			if (jcInfo->has("longmarginratio"))
				lMargin = jcInfo->getDouble("longmarginratio");
			if (jcInfo->has("shortmarginratio"))
				sMargin = jcInfo->getDouble("shortmarginratio");
			cInfo->setMarginRatios(lMargin, sMargin);

			WTSContractList* contractList = (WTSContractList*)m_mapExchgContract->get(ShortKey(cInfo->getExchg()));
			if (contractList == NULL)
			{
				contractList = WTSContractList::create();
				m_mapExchgContract->add(ShortKey(cInfo->getExchg()), contractList, false);
			}
			contractList->add(LongKey(cInfo->getSymbol()), cInfo, false);

			LongKey key = LongKey(cInfo->getSymbol());
			WTSArray* ayInst = (WTSArray*)m_mapContracts->get(key);
			if(ayInst == NULL)
			{
				ayInst = WTSArray::create();
				m_mapContracts->add(key, ayInst, false);
			}

			ayInst->append(cInfo, true);
		}
	}

	WTSLogger::info("Contracts configuration file {} loaded, {} exchanges", http_url, m_mapExchgContract->size());
	root->release();
	return true;
}
