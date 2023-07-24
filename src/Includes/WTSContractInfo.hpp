/*!
 * \file WTSContractInfo.hpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 *
 * \brief Wt品种信息、合约信息定义文件
 */
#pragma once
#include "WTSObject.hpp"
#include "WTSTypes.h"
#include "FasterDefs.h"
#include <string>
#include <sstream>

NS_WTP_BEGIN
class WTSSessionInfo;

class WTSCommodityInfo: public WTSObject
{
public:
	static WTSCommodityInfo* create(const char* pid, const char* name, const char* exchg, const char* session, const char* trdtpl, const char* currency = "CNY")
	{
		WTSCommodityInfo* ret = new WTSCommodityInfo;
		ret->m_strName = name;
		ret->m_strExchg = exchg;
		ret->m_strProduct = pid;
		ret->m_strCurrency = currency;
		ret->m_strSession = session;
		ret->m_strTrdTpl = trdtpl;

		std::stringstream ss;
		ss << exchg << "." << pid;
		ret->m_strFullPid = ss.str();

		return ret;
	}

	inline void	setVolScale(uint32_t volScale){ m_uVolScale = volScale; }
	inline void	setPriceTick(double pxTick){ m_dPriceTick = pxTick; }
	inline void	setCategory(ContractCategory cat){ m_ccCategory = cat; }
	inline void	setCoverMode(CoverMode cm){ m_coverMode = cm; }
	inline void	setPriceMode(PriceMode pm){ m_priceMode = pm; }
	inline void	setTradingMode(TradingMode tm) { m_tradeMode = tm; }

	inline bool canShort() const { return m_tradeMode == TM_Both; }
	inline bool isT1() const { return m_tradeMode == TM_LongT1; }

	inline const char* getName()	const{ return m_strName.c_str(); }
	inline const char* getExchg()	const{ return m_strExchg.c_str(); }
	inline const char* getProduct()	const{ return m_strProduct.c_str(); }
	inline const char* getCurrency()	const{ return m_strCurrency.c_str(); }
	inline const char* getSession()	const{ return m_strSession.c_str(); }
	inline const char* getTradingTpl()	const{ return m_strTrdTpl.c_str(); }
	inline const char* getFullPid()	const{ return m_strFullPid.c_str(); }

	inline uint32_t	getVolScale()	const{ return m_uVolScale; }
	inline double	getPriceTick()	const{ return m_dPriceTick; }
	//inline uint32_t	getPrecision()	const{ return m_uPrecision; }

	inline ContractCategory		getCategoty() const{ return m_ccCategory; }
	inline CoverMode			getCoverMode() const{ return m_coverMode; }
	inline PriceMode			getPriceMode() const{ return m_priceMode; }
	inline TradingMode			getTradingMode() const { return m_tradeMode; }

	inline void		addCode(const char* code){ m_setCodes.insert(code); }
	inline const CodeSet& getCodes() const{ return m_setCodes; }

	inline void	setLotsTick(double lotsTick){ m_dLotTick = lotsTick; }
	inline void	setMinLots(double minLots) { m_dMinLots = minLots; }

	inline bool isOption() const
	{
		return (m_ccCategory == CC_FutOption || m_ccCategory == CC_ETFOption || m_ccCategory == CC_SpotOption);
	}

	inline bool isFuture() const
	{
		return m_ccCategory == CC_Future;
	}

	inline bool isStock() const
	{
		return m_ccCategory == CC_Stock;
	}

	inline double	getLotsTick() const { return m_dLotTick; }
	inline double	getMinLots() const { return m_dMinLots; }

	inline void		setSessionInfo(WTSSessionInfo* sInfo) { m_pSession = sInfo; }
	inline WTSSessionInfo* getSessionInfo() const { return m_pSession; }

private:
	std::string	m_strName;		//品种名称
	std::string	m_strExchg;		//交易所代码
	std::string	m_strProduct;	//品种ID
	std::string	m_strCurrency;	//币种
	std::string m_strSession;	//交易时间模板
	std::string m_strTrdTpl;	//节假日模板
	std::string m_strFullPid;	//全品种ID，如CFFEX.IF

	uint32_t	m_uVolScale;	//合约放大倍数
	double		m_dPriceTick;	//最小价格变动单位
	//uint32_t	m_uPrecision;	//价格精度

	double		m_dLotTick;		//数量精度
	double		m_dMinLots;		//最小交易数量

	ContractCategory	m_ccCategory;	//品种分类，期货、股票、期权等
	CoverMode			m_coverMode;	//平仓类型
	PriceMode			m_priceMode;	//价格类型
	TradingMode			m_tradeMode;	//交易类型

	CodeSet				m_setCodes;

	WTSSessionInfo*		m_pSession;
};

class WTSContractInfo :	public WTSObject
{
public:
	static WTSContractInfo* create(const char* symbol, const char* exchg, const char* base, const char* quote, const char* margin)
	{
		WTSContractInfo* ret = new WTSContractInfo;
		ret->m_strSymbol = symbol;
		ret->m_strExchg = exchg;
		ret->m_strBaseAsset = base;
		ret->m_strQuoteAsset = quote;
		ret->m_strMarginAsset = margin;

		return ret;
	}


	inline void	setVolumeLimits(double maxMarketVol, double maxLimitVol, double minMarketVol = 1, double minLimitVol = 1)
	{
		m_maxMktQty = maxMarketVol;
		m_maxLmtQty = maxLimitVol;

		m_minLmtQty = minLimitVol;
		m_minMktQty = minMarketVol;
	}

	inline void setDates(uint64_t openDate, uint64_t expireDate)
	{
		m_openDate = openDate;
		m_expireDate = expireDate;
	}

	inline void setMarginRatios(double maintRatio, double requiredRatio)
	{
		m_maintMarginRatio = maintRatio;
		m_requiredMarginRatio = requiredRatio;
	}

	inline void setPriceBounds(double min, double max, double tick)
	{
		m_minPrice = min;
		m_maxPrice = max;
		m_dPriceTick = tick;
	}

	inline void setLotBounds(double lotSize, double minSize, double minNotional)
	{
		m_dLotTick = lotSize;
		m_dMinLots = minSize;
		m_minNotional = minNotional;
	}

	inline void setPercentPrice(double multiplierDown, double multiplierUp)
	{
		m_multiplierDown = multiplierDown;
		m_multiplierUp = multiplierUp;
	}

	inline const char* getSymbol()	const{return m_strSymbol.c_str();}
	inline const char* getExchg()	    const{return m_strExchg.c_str();}
	inline const char* getBaseAsset()	const{return m_strBaseAsset.c_str();}
	inline const char* getQuoteAsset()	const{return m_strQuoteAsset.c_str();}
	inline const char* getMarginAsset()	const{return m_strMarginAsset.c_str();}

	inline ContractCategory		getCategoty() const{ return m_ccCategory; }
	inline void		setCategoty(ContractCategory cc) { m_ccCategory = cc; }

	inline double	getMaintMarginRatio()    const { return m_maintMarginRatio; }
	inline double	getRequiredMarginRatio() const { return m_requiredMarginRatio; }

	inline double	getPriceTick()	const{ return m_dPriceTick; }
	inline uint32_t getPrecision()  const{ return m_uPrecision; }

	inline double	getLotsTick() const { return m_dLotTick; }
	inline double	getMinLots() const { return m_dMinLots; }
	inline double	getStepSize() const { return m_dStepSize; }

	inline double	getMaxMktVol() const{ return m_maxMktQty; }
	inline double	getMaxLmtVol() const{ return m_maxLmtQty; }
	inline double	getMinMktVol() const { return m_minMktQty; }
	inline double	getMinLmtVol() const { return m_minLmtQty; }
	inline double	getMaxPrice()  const { return m_maxPrice; }
	inline double	getMinPrice()  const { return m_minPrice; }
	inline double	getMinNotional()  const { return m_minNotional; }
	inline double	getMultiplierDown() const { return m_multiplierDown; }
	inline double	getMultiplierUp() const { return m_multiplierUp; }

	inline uint32_t	getOpenDate() const { return m_openDate; }
	inline uint32_t	getExpireDate() const { return m_expireDate; }


protected:
	WTSContractInfo() {}
	virtual ~WTSContractInfo(){}

private:
	std::string	m_strSymbol;	//交易对名称
	std::string	m_strExchg;
	std::string	m_strBaseAsset;
	std::string	m_strQuoteAsset;
	std::string	m_strMarginAsset;
	ContractCategory m_ccCategory;	//合约分类，期货、股票、期权等

	double		m_maintMarginRatio;		//维持保证金率
	double		m_requiredMarginRatio;	//初始保证金率

	// filters 限制参数
	double		m_dPriceTick;	//最小价格变动单位
	uint32_t	m_uPrecision{0};	//价格精度

	double		m_dLotTick;		//数量精度
	double		m_dMinLots;		//最小交易数量
	double		m_dStepSize;    //数量变动步进值

	double		m_maxMktQty;	//市价单最大数量
	double		m_maxLmtQty;	//限价单最大数量
	double		m_minMktQty;	//市价单最小数量
	double		m_minLmtQty;	//限价单最小数量
	double 		m_maxPrice;		//最大允许价格
	double		m_minPrice;		//最小允许价格
	double		m_minNotional; 	//最小名义价值

	// 基于标记价格计算的挂单价格的可接受区间，挂单价格必须同时满足以下条件：
	// 买单: price <= markPrice * multiplierUp
	// 卖单: price >= markPrice * multiplierDown
	double		m_multiplierDown;
	double		m_multiplierUp;

	uint64_t	m_openDate{0};		//上市日期
	uint64_t	m_expireDate{0};	//交割日


	WTSCommodityInfo*	m_commInfo;
};


NS_WTP_END