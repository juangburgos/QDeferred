#include "qlambdathreadworkerdata.h"
#include <QCoreApplication>
#include <sstream>

QLambdaThreadWorkerDataEvent::QLambdaThreadWorkerDataEvent() : QEvent(QLAMBDATHREADWORKERDATA_EVENT_TYPE)
{
	// nothing to do here
}

QLambdaThreadWorkerObjectData::QLambdaThreadWorkerObjectData() : QObject(nullptr)
{
	// nothing to do here either
}

void QLambdaThreadWorkerObjectData::timerEvent(QTimerEvent *event)
{
	// get timer id and execute related function
	int timerId = event->timerId();
	// check if map contains such timer, of not return
	if (!m_mapFuncs.contains(timerId))
	{
		return;
	}
	// if timer exists, exec function
	m_mapFuncs[timerId]();
}

bool QLambdaThreadWorkerObjectData::event(QEvent * ev)
{
	if (ev->type() == QLAMBDATHREADWORKERDATA_EVENT_TYPE) {
		// call function
		static_cast<QLambdaThreadWorkerDataEvent*>(ev)->m_eventFunc();
		// return event processed
		return true;
	}
	// Call base implementation (make sure the rest of events are handled)
	return QObject::event(ev);
}

QLambdaThreadWorkerData::QLambdaThreadWorkerData()
{
	mp_workerThread = new QThread;
	mp_workerObj    = new QLambdaThreadWorkerObjectData;
	// initial value
	m_intIdCounter  = 0;
	// get thread id
	std::stringstream stream;
	stream << std::hex << (size_t)mp_workerThread;
	m_strThreadId = "QThread(0x" + QString::fromStdString(stream.str()) + ")";
	// move worker object to thread and subscribe for deletion
	mp_workerObj->moveToThread(mp_workerThread);
	QObject::connect(mp_workerThread, &QThread::finished, mp_workerObj, &QObject::deleteLater);
	QObject::connect(mp_workerThread, SIGNAL(finished()), mp_workerThread, SLOT(deleteLater()));
	// start thread
	mp_workerThread->start();
}

QLambdaThreadWorkerData::QLambdaThreadWorkerData(const QLambdaThreadWorkerData &other) : QSharedData(other),
mp_workerObj(other.mp_workerObj)
{
	this->mp_workerThread = other.mp_workerThread; 
	this->m_strThreadId   = other.m_strThreadId;
	this->m_intIdCounter  = other.m_intIdCounter;
}

QLambdaThreadWorkerData::~QLambdaThreadWorkerData()
{
	mp_workerThread->quit();
}

void QLambdaThreadWorkerData::execInThread(std::function<void()> &threadFunc)
{
	// create event to exec in thread
	QLambdaThreadWorkerDataEvent * p_Evt = new QLambdaThreadWorkerDataEvent;
	p_Evt->m_eventFunc = threadFunc;
	// post event to thread
	QCoreApplication::postEvent(mp_workerObj, p_Evt);
}

QString QLambdaThreadWorkerData::getThreadId()
{
	return m_strThreadId;
}

int QLambdaThreadWorkerData::startLoopInThread(std::function<void()> &threadLoopFunc, int intMsSleep /*= 1000*/)
{
	// limit
	if (intMsSleep < 0)
	{
		intMsSleep = 0;
	}
	// get new custom id
	m_intIdCounter++;
	int newLoopId = m_intIdCounter;
	// create function to start loop
	std::function<void()> funcStartLoop = [this, threadLoopFunc, intMsSleep, newLoopId]() {
		// start the timer
		int timerId = mp_workerObj->startTimer(intMsSleep);
		// add timer id to map
		m_mapIdtimerIds[newLoopId] = timerId;
		// add function to map
		mp_workerObj->m_mapFuncs[timerId] = threadLoopFunc;
	};
	// serialize map access by using event queue
	this->execInThread(funcStartLoop);
	// return loop id (we still do not have timerId)
	return newLoopId;
}

bool QLambdaThreadWorkerData::stopLoopInThread(const int &intLoopId)
{
	// check if exists, if not, return false
	if (!m_mapIdtimerIds.contains(intLoopId))
	{
		return false;
	}
	// 
	int timerId = m_mapIdtimerIds.take(intLoopId);
	// create function to stop loop
	auto pWorkerObjCopy = mp_workerObj;
	std::function<void()> funcStopLoop = [pWorkerObjCopy, timerId]() {
		// if exists, stop timer
		pWorkerObjCopy->killTimer(timerId);
		// remove function from map
		pWorkerObjCopy->m_mapFuncs.remove(timerId);
	};
	// serialize map access by using event queue
	this->execInThread(funcStopLoop);
	// return success
	return true;
}
