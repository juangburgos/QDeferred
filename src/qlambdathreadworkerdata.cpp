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

void QLambdaThreadWorkerData::execInThread(const std::function<void()> &threadFunc, const Qt::EventPriority &priority/* = Qt::NormalEventPriority*/)
{
	// create event to exec in thread
	QLambdaThreadWorkerDataEvent * p_Evt = new QLambdaThreadWorkerDataEvent;
	p_Evt->m_eventFunc = threadFunc;
	// post event to thread
	QCoreApplication::postEvent(mp_workerObj, p_Evt, priority);
}

QString QLambdaThreadWorkerData::getThreadId()
{
	return m_strThreadId;
}

QThread * QLambdaThreadWorkerData::getThread()
{
	return mp_workerThread;
}

int QLambdaThreadWorkerData::startLoopInThread(const std::function<void()> &threadLoopFunc, const quint32 &intMsSleep /*= 1000*/)
{
	// get new custom id
	m_intIdCounter++;
	int newLoopId = m_intIdCounter;
	// create function to start loop
	// serialize map access by using event queue
	this->execInThread([this, threadLoopFunc, intMsSleep, newLoopId]() {
		// start the timer
		int timerId = mp_workerObj->startTimer(intMsSleep);
		// add timer id to map
		m_mapIdtimerIds[newLoopId] = timerId;
		// add function to map
		mp_workerObj->m_mapFuncs[timerId] = threadLoopFunc;
	});
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
	// create function to stop loop
	// serialize map access by using event queue
	this->execInThread([this, intLoopId]() {
		// get real timer id of timer to stop
		int timerId = m_mapIdtimerIds.take(intLoopId);
		// stop timer
		mp_workerObj->killTimer(timerId);
		// remove function from map
		mp_workerObj->m_mapFuncs.remove(timerId);
	});
	// return success
	return true;
}

void QLambdaThreadWorkerData::moveQObjectToThread(QObject * pObject)
{
	pObject->moveToThread(mp_workerThread);
}
