/*!
 * \file IBaseDataMgr.h
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 *
 * \brief 基础数据管理器接口定义
 */
#pragma once
#include <string>
#include <stdint.h>

#include "WTSMarcos.h"
#include "FasterDefs.h"

NS_WTP_BEGIN
typedef CodeSet ContractSet;

class WTSContractInfo;
class WTSArray;

class IBaseDataMgr
{
public:
	virtual WTSContractInfo*	getContract(const char* code, const char* exchg = "")	= 0;
	virtual WTSArray*			getContracts(const char* exchg = "")					= 0;
};
NS_WTP_END