#pragma once
#include "WTSMarcos.h"
#include "WTSTypes.h"
#include <string>

NS_WTP_BEGIN
class WTSVariant;
class WTSTickData;
class IHotMgr;
class IBaseDataMgr;

class IIndexContext
{
public:
	IIndexContext(){}
	virtual ~IIndexContext() {}

public:
	virtual IHotMgr*		get_hot_mgr() = 0;
	virtual IBaseDataMgr*	get_bd_mgr() = 0;

	virtual WTSTickData*	sub_ticks(const char* fullCode) = 0;

	virtual WTSTickData*	get_tick(const char* code, const char* exchg) = 0;

	virtual void			push_tick(WTSTickData* newTick) = 0;

	virtual void			output_log(WTSLogLevel ll, const char* message) = 0;
};

class IIndexWorker
{
public:
	IIndexWorker(const char* id):_factory(nullptr), _hot_mgr(nullptr), _bd_mgr(nullptr), _id(id){}
	virtual ~IIndexWorker(){}

public:
	/*
	 *	��ȡ������
	 */
	virtual const char* get_fact_name() = 0;

	virtual bool	init(WTSVariant* config) = 0;

	virtual void	handle_quote(WTSTickData* newTick) = 0;

	void	set_factory(IIndexContext* factory)
	{
		_factory = factory;
		if(_factory)
		{
			_hot_mgr = _factory->get_hot_mgr();
			_bd_mgr = _factory->get_bd_mgr();
		}
	}

protected:
	IIndexContext*	_factory;
	IHotMgr*		_hot_mgr;
	IBaseDataMgr*	_bd_mgr;
	std::string		_id;
};

//////////////////////////////////////////////////////////////////////////
//���Թ����ӿ�
typedef void(*FuncEnumIndexWorkerCallback)(const char* factName, const char* straName, bool isLast);

class IIndexWorkerFact
{
public:
	IIndexWorkerFact() {}
	virtual ~IIndexWorkerFact() {}

public:
	/*
	 *	��ȡ������
	 */
	virtual const char* get_name() = 0;

	/*
	 *	��������ָ��������
	 */
	virtual IIndexWorker* create_worker(const char* name, const char* id) = 0;


	/*
	 *	ɾ��ָ��������
	 */
	virtual bool delete_worker(IIndexWorker* stra) = 0;
};


NS_WTP_END

//��������
typedef wtp::IIndexWorkerFact* (*FuncCreateIndexFact)();
//ɾ������
typedef void(*FuncDeleteIndexFact)(wtp::IIndexWorkerFact* &fact);
