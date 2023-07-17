/*!
 * \file StateMonitor.cpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief 
 */
#include "StateMonitor.h"
#include "DataManager.h"

#include "../Share/TimeUtils.hpp"
#include "../Includes/WTSContractInfo.hpp"
#include "../Includes/WTSSessionInfo.hpp"
#include "../Includes/WTSVariant.hpp"

#include "../WTSTools/WTSBaseDataMgr.h"
#include "../WTSTools/WTSLogger.h"
#include "../WTSUtils/WTSCfgLoader.h"


StateMonitor::StateMonitor()
	: _stopped(false)
	, _bd_mgr(NULL)
	, _dt_mgr(NULL)
{
}


StateMonitor::~StateMonitor()
{
}

bool StateMonitor::initialize(const char* filename, WTSBaseDataMgr* bdMgr, DataManager* dtMgr)
{
	_bd_mgr = bdMgr;
	_dt_mgr = dtMgr;

	if (!StdFile::exists(filename))
	{
		WTSLogger::error("State config file {} not exists", filename);
		return false;
	}

	WTSVariant* config = WTSCfgLoader::load_from_file(filename);
	if (config == NULL)
	{
		WTSLogger::error("Loading state config failed");
		return false;
	}

	auto keys = config->memberNames();
	for (const std::string& sid : keys)
	{
		WTSVariant* jItem = config->get(sid.c_str());

		WTSSessionInfo* ssInfo = _bd_mgr->getSession(sid.c_str());
		if (ssInfo == NULL)
		{
			WTSLogger::error("Trading session template [{}] not exists,state control rule skipped", sid);
			continue;
		}

		StatePtr sInfo(new StateInfo);
		sInfo->_init_time = jItem->getUInt32("inittime");	//初始化时间,初始化以后数据才开始接收
		sInfo->_close_time = jItem->getUInt32("closetime");	//收盘时间,收盘后数据不再接收了
		sInfo->_proc_time = jItem->getUInt32("proctime");	//盘后处理时间,主要把实时数据转到历史去

		strcpy(sInfo->_session, sid.c_str());

		const auto& auctions = ssInfo->getAuctionSections();//这里面是偏移过的时间,要注意了!!!
		for(const auto& secInfo : auctions)
		{
			uint32_t stime = secInfo.first;
			uint32_t etime = secInfo.second;

			stime = stime / 100 * 60 + stime % 100;//先将时间转成分钟数
			etime = etime / 100 * 60 + etime % 100;

			stime = stime / 60 * 100 + stime % 60;//再将分钟数转成时间
			etime = etime / 60 * 100 + etime % 60;//先不考虑半夜12点的情况,目前看来,几乎没有
			sInfo->_sections.emplace_back(StateInfo::Section({ stime, etime }));
		}

		const auto& sections = ssInfo->getTradingSections();//这里面是偏移过的时间,要注意了!!!
		for (const auto& secInfo : sections)
		{
			uint32_t stime = secInfo.first;
			uint32_t etime = secInfo.second;

			stime = stime / 100 * 60 + stime % 100;//先将时间转成分钟数
			etime = etime / 100 * 60 + etime % 100;

			stime--;//开始分钟数-1
			etime++;//结束分钟数+1

			stime = stime / 60 * 100 + stime % 60;//再将分钟数转成时间
			etime = etime / 60 * 100 + etime % 60;//先不考虑半夜12点的情况,目前看来,几乎没有
			sInfo->_sections.emplace_back(StateInfo::Section({ stime, etime }));
		}

		_map[sInfo->_session] = sInfo;

		CodeSet* pCommSet =  _bd_mgr->getSessionComms(sInfo->_session);
		if (pCommSet)
		{
			uint32_t curDate = TimeUtils::getCurDate();
			uint32_t curMin = TimeUtils::getCurMin() / 100;
			uint32_t offDate = ssInfo->getOffsetDate(curDate, curMin);
			uint32_t offMin = ssInfo->offsetTime(curMin, true);

			//先获取基准的交易日

			for (auto it = pCommSet->begin(); it != pCommSet->end(); it++)
			{
				const char* pid = (*it).c_str();

				 _bd_mgr->setTradingDate(pid,  _bd_mgr->getTradingDate(pid, offDate, offMin, false), false);
				uint32_t prevDate = TimeUtils::getNextDate(curDate, -1);
				if ((ssInfo->getOffsetMins() > 0 &&
					(! _bd_mgr->isTradingDate(pid, curDate) &&
					!(ssInfo->isInTradingTime(curMin) &&  _bd_mgr->isTradingDate(pid, prevDate)))) ||
					(ssInfo->getOffsetMins() <= 0 && ! _bd_mgr->isTradingDate(pid, offDate))
					)
				{
					WTSLogger::info("Instrument {} is in holiday", pid);
				}
			}
		}
	}
	return true;
}

void StateMonitor::run()
{
	if(_thrd == NULL)
	{
		_thrd.reset(new StdThread([this](){

			while (!_stopped)
			{
				static uint64_t lastTime = 0;

				while(true)
				{
					uint64_t now = TimeUtils::getLocalTimeNow();
					if(now - lastTime >= 1000)
						break;

					if(_stopped)
						break;

					std::this_thread::sleep_for(std::chrono::milliseconds(2));
				}

				if(_stopped)
					break;

				uint32_t curDate = TimeUtils::getCurDate();
				uint32_t curMin = TimeUtils::getCurMin() / 100;

				auto it = _map.begin();
				for (; it != _map.end(); it++)
				{
					StatePtr& sInfo = (StatePtr&)it->second;
					WTSSessionInfo* mInfo =  _bd_mgr->getSession(sInfo->_session);

					uint32_t offDate = mInfo->getOffsetDate(curDate, curMin);
					uint32_t prevDate = TimeUtils::getNextDate(curDate, -1);

					switch(sInfo->_state)
					{
					case SS_ORIGINAL:
						{
							uint32_t offTime = mInfo->offsetTime(curMin, true);
							uint32_t offInitTime = mInfo->offsetTime(sInfo->_init_time, true);
							uint32_t offCloseTime = mInfo->offsetTime(sInfo->_close_time, false);
							uint32_t aucStartTime = mInfo->getAuctionStartTime(true);

							bool isAllHoliday = true;
							std::stringstream ss_a, ss_b;
							CodeSet* pCommSet =  _bd_mgr->getSessionComms(sInfo->_session);
							if (pCommSet)
							{
								for (auto it = pCommSet->begin(); it != pCommSet->end(); it++)
								{
									const char* pid = (*it).c_str();
									/*
									 *	如果时间往后偏移
									 *	如果当前日期不是交易日,且不处于夜盘后半夜（交易时间且昨天是交易日）
									 *	或者时间往后偏移的话,就看偏移日期是否是节假日
									 */
									if ((mInfo->getOffsetMins() > 0 &&
										(! _bd_mgr->isTradingDate(pid, curDate) &&	//当前日志不是交易日
										!(mInfo->isInTradingTime(curMin) &&  _bd_mgr->isTradingDate(pid, prevDate)))) ||	//当前不在交易时间,且昨天是交易日
										(mInfo->getOffsetMins() <= 0 && ! _bd_mgr->isTradingDate(pid, offDate))
										)
									{
										ss_a << pid << ",";
										WTSLogger::info("Instrument {} is in holiday", pid);
									}
									else
									{
										ss_b << pid << ",";
										isAllHoliday = false;
									}
								}

							}
							else
							{
								WTSLogger::info("No corresponding instrument of trading session {}[{}], changed into holiday state", mInfo->name(), sInfo->_session);
								sInfo->_state = SS_Holiday;
							}

							if(isAllHoliday)
							{
								WTSLogger::info("All instruments of trading session {}[{}] are in holiday, changed into holiday state", mInfo->name(), sInfo->_session);
								sInfo->_state = SS_Holiday;
							}
							else if (offTime >= offCloseTime)
							{
								sInfo->_state = SS_CLOSED;
								WTSLogger::info("Trading session {}[{}] stopped receiving data", mInfo->name(), sInfo->_session);
							}
							else if (aucStartTime != -1 && offTime >= aucStartTime)
							{
								if (sInfo->isInSections(offTime))
								{
									//if(sInfo->_schedule)
									//{
									//	_dt_mgr->preloadRtCaches();
									//}
									sInfo->_state = SS_RECEIVING;
									WTSLogger::info("Trading session {}[{}] started receiving data", mInfo->name(), sInfo->_session);
								}
								else
								{
									//小于市场收盘时间,且不在交易时间,则为中途休盘时间
									if(offTime < mInfo->getCloseTime(true))
									{
										sInfo->_state = SS_PAUSED;
										WTSLogger::info("Trading session {}[{}] paused receiving data", mInfo->name(), sInfo->_session);
									}
									else
									{//大于市场收盘时间,但是没有大于接收收盘时间,则还要继续接收,主要是要收结算价
										sInfo->_state = SS_RECEIVING;
										WTSLogger::info("Trading session {}[{}] started receiving data", mInfo->name(), sInfo->_session);
									}
									
								}
							}								
							else if (offTime >= offInitTime)
							{
								sInfo->_state = SS_INITIALIZED;
								WTSLogger::info("Trading session {}[{}] initialized", mInfo->name(), sInfo->_session);
							}

							
						}
						break;
					case SS_INITIALIZED:
						{
							uint32_t offTime = mInfo->offsetTime(curMin, true);
							uint32_t offAucSTime = mInfo->getAuctionStartTime(true);
							if (offAucSTime == -1 || offTime >= mInfo->getAuctionStartTime(true))
							{
								if (!sInfo->isInSections(offTime) && offTime < mInfo->getCloseTime(true))
								{
									//if (sInfo->_schedule)
									//{
									//	_dt_mgr->preloadRtCaches();
									//}
									sInfo->_state = SS_PAUSED;

									WTSLogger::info("Trading session {}[{}] paused receiving data", mInfo->name(), sInfo->_session);
								}
								else
								{
									//if (sInfo->_schedule)
									//{
									//	_dt_mgr->preloadRtCaches();
									//}
									sInfo->_state = SS_RECEIVING;
									WTSLogger::info("Trading session {}[{}] started receiving data", mInfo->name(), sInfo->_session);
								}
								
							}
						}
						break;
					case SS_RECEIVING:
						{
							uint32_t offTime = mInfo->offsetTime(curMin, true);
							uint32_t offCloseTime = mInfo->offsetTime(sInfo->_close_time, false);
							if (offTime >= offCloseTime)
							{
								sInfo->_state = SS_CLOSED;

								WTSLogger::info("Trading session {}[{}] stopped receiving data", mInfo->name(), sInfo->_session);
							}
							else if (offTime >= mInfo->getAuctionStartTime(true))
							{
								if (offTime < mInfo->getCloseTime(true))
								{
									if (!sInfo->isInSections(offTime))
									{
										//if (sInfo->_schedule)
										//{
										//	_dt_mgr->preloadRtCaches();
										//}
										sInfo->_state = SS_PAUSED;

										WTSLogger::info("Trading session {}[{}] paused receiving data", mInfo->name(), sInfo->_session);
									}
								}
								else
								{
									//这就是下午收盘以后的时间
									//这里不能改状态,因为要收结算价
								}
							}
						}
						break;
					case SS_PAUSED:
						{
							//休息状态只能转换为交易状态
							//这里要用偏移过的日期,不然如果周六早上有中途休息,就会出错
							uint32_t weekDay = TimeUtils::getWeekDay();

							bool isAllHoliday = true;
							CodeSet* pCommSet =  _bd_mgr->getSessionComms(sInfo->_session);
							if (pCommSet)
							{
								for (auto it = pCommSet->begin(); it != pCommSet->end(); it++)
								{
									const char* pid = (*it).c_str();
									if ((mInfo->getOffsetMins() > 0 &&
										(! _bd_mgr->isTradingDate(pid, curDate) &&
										!(mInfo->isInTradingTime(curMin) &&  _bd_mgr->isTradingDate(pid, prevDate)))) ||
										(mInfo->getOffsetMins() <= 0 && ! _bd_mgr->isTradingDate(pid, offDate))
										)
									{
										WTSLogger::info("Instrument {} is in holiday", pid);
									}
									else
									{
										isAllHoliday = false;
									}
								}
							}
							
							if (!isAllHoliday)
							{
								uint32_t offTime = mInfo->offsetTime(curMin, true);
								if (sInfo->isInSections(offTime))
								{
									sInfo->_state = SS_RECEIVING;
									WTSLogger::info("Trading session {}[{}] continued to receive data", mInfo->name(), sInfo->_session);
								}
							}
							else
							{
								WTSLogger::info("All instruments of trading session {}[{}] are in holiday, changed into holiday state", mInfo->name(), sInfo->_session);
								sInfo->_state = SS_Holiday;
							}
						}
						break;
					case SS_CLOSED:
						{
							uint32_t offTime = mInfo->offsetTime(curMin, true);
							uint32_t offProcTime = mInfo->offsetTime(sInfo->_proc_time, true);
							if (offTime >= offProcTime)
							{
								if(!_dt_mgr->isSessionProceeded(sInfo->_session))
								{
									sInfo->_state = SS_PROCING;

									WTSLogger::info("Trading session {}[{}] started processing closing task", mInfo->name(), sInfo->_session);
									_dt_mgr->transHisData(sInfo->_session);
								}
								else
								{
									sInfo->_state = SS_PROCED;
								}
							}
							else if (offTime >= mInfo->getAuctionStartTime(true) && offTime <= mInfo->getCloseTime(true))
							{
								if (!sInfo->isInSections(offTime))
								{
									sInfo->_state = SS_PAUSED;

									WTSLogger::info("Trading session {}[{}] paused receiving data", mInfo->name(), sInfo->_session);
								}
							}
						}
						break;
					case SS_PROCING:
						sInfo->_state = SS_PROCED;
						break;
					case SS_PROCED:
					case SS_Holiday:
						{
							uint32_t offTime = mInfo->offsetTime(curMin, true);
							uint32_t offInitTime = mInfo->offsetTime(sInfo->_init_time, true);
							if (offTime >= 0 && offTime < offInitTime)
							{
								bool isAllHoliday = true;
								CodeSet* pCommSet =  _bd_mgr->getSessionComms(sInfo->_session);
								if (pCommSet)
								{
									for (auto it = pCommSet->begin(); it != pCommSet->end(); it++)
									{
										const char* pid = (*it).c_str();
										if ((mInfo->getOffsetMins() > 0 &&
											(! _bd_mgr->isTradingDate(pid, curDate) &&
											!(mInfo->isInTradingTime(curMin) &&  _bd_mgr->isTradingDate(pid, prevDate)))) ||
											(mInfo->getOffsetMins() <= 0 && ! _bd_mgr->isTradingDate(pid, offDate))
											)
										{
											
										}
										else
										{
											isAllHoliday = false;
										}
									}
								}

								if(!isAllHoliday)
								{
									sInfo->_state = SS_ORIGINAL;
									WTSLogger::info("Trading session {}[{}] state reset", mInfo->name(), sInfo->_session);
								}
							}
						}
						break;
					}
					
				}

				lastTime = TimeUtils::getLocalTimeNow();

				if (isAllInState(SS_PROCING) && !isAllInState(SS_Holiday))
				{
					//缓存清理
					_dt_mgr->transHisData("CMD_CLEAR_CACHE");
				}
			}
		}));
	}
}

void StateMonitor::stop()
{
	_stopped = true;

	if (_thrd)
		_thrd->join();
}

bool StateMonitor::isAllInState(SimpleState ss) const
{
	auto it = _map.begin();
	for (; it != _map.end(); it++)
	{
		const StatePtr& sInfo = it->second;
		if (sInfo->_state != SS_Holiday && sInfo->_state != ss)
			return false;
	}

	return true;
}


bool StateMonitor::isAnyInState(SimpleState ss) const
{
	auto it = _map.begin();
	for (; it != _map.end(); it++)
	{
		const StatePtr& sInfo = it->second;
		if (sInfo->_state == ss)
			return true;
	}

	return false;
}

bool StateMonitor::isInState(const char* sid, SimpleState ss) const
{
	auto it = _map.find(sid);
	if (it == _map.end())
		return false;

	const StatePtr& sInfo = it->second;
	return sInfo->_state == ss;
}