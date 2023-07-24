/*!
 * \file WTSBaseDataMgr.h
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 *
 * \brief 基础数据管理器实现
 */
#pragma once
#include "../Includes/IBaseDataMgr.h"
#include "../Includes/WTSCollection.hpp"
#include "../Includes/FasterDefs.h"

USING_NS_WTP;

typedef WTSHashMap<LongKey>		WTSContractList;
typedef WTSHashMap<ShortKey>	WTSExchgContract;
typedef WTSHashMap<LongKey>		WTSContractMap;


class WTSBaseDataMgr : public IBaseDataMgr
{
public:
	WTSBaseDataMgr();
	~WTSBaseDataMgr();

public:
	virtual WTSContractInfo*	getContract(const char* code, const char* exchg = "") override;
	virtual WTSArray*			getContracts(const char* exchg = "") override;

	bool loadContracts(const char* http_url);
	void release();

private:
	WTSExchgContract*	m_mapExchgContract;
	WTSContractMap*		m_mapContracts;
};

