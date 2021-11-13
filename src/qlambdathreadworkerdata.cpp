#include "qlambdathreadworkerdata.h"
#include <QCoreApplication>
#include <sstream>

// QDEFTHREADWORKERDATAEVENT -------------------------------------------------

QLambdaThreadWorkerDataEvent::QLambdaThreadWorkerDataEvent() : QEvent(QLAMBDATHREADWORKERDATA_EVENT_TYPE)
{
	// nothing to do here
}

// QDEFTHREADWORKEROBJECTDATA -----------------------------------------------------

QLambdaThreadWorkerObjectData::QLambdaThreadWorkerObjectData() : 
	QObject(nullptr),
	m_callbacksToExec(0)
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
		// decrement callback count
		this->decrementCallbackCount();
		// return event processed
		return true;
	}
	// Call base implementation (make sure the rest of events are handled)
	return QObject::event(ev);
}

quint32 QLambdaThreadWorkerObjectData::incrementCallbackCount()
{
	QMutexLocker locker(&m_mutex);
	return ++m_callbacksToExec;
}

quint32 QLambdaThreadWorkerObjectData::decrementCallbackCount()
{
	m_mutex.lock();
	--m_callbacksToExec;
	m_mutex.unlock();
	if (m_callbacksToExec == 0)
	{
		emit this->finishedProcessingCallbacks();
	}
	return m_callbacksToExec;
}

// QDEFTHREADWORKERDATA -----------------------------------------------------

QLambdaThreadWorkerData::QLambdaThreadWorkerData()
{
	mp_workerThread = new QThread;
	mp_workerObj    = new QLambdaThreadWorkerObjectData;
	// initial value
	m_requestedQuit = false;
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

QLambdaThreadWorkerData::QLambdaThreadWorkerData(const QLambdaThreadWorkerData &other) : 
	QSharedData(other),
	mp_workerThread(other.mp_workerThread),
	mp_workerObj   (other.mp_workerObj   ),
	m_strThreadId  (other.m_strThreadId  ),
	m_intIdCounter (other.m_intIdCounter ),
	m_requestedQuit(other.m_requestedQuit)
{

}

QLambdaThreadWorkerData::~QLambdaThreadWorkerData()
{
	if (m_requestedQuit || mp_workerThread->isFinished())
	{
		return;
	}
	mp_workerThread->quit();
}

bool QLambdaThreadWorkerData::execInThread(const std::function<void()> &threadFunc, const Qt::EventPriority &priority/* = Qt::NormalEventPriority*/)
{
	if (!mp_workerThread->isRunning() || m_requestedQuit)
	{
		return false;
	}
	// create event to exec in thread
	QLambdaThreadWorkerDataEvent * p_Evt = new QLambdaThreadWorkerDataEvent;
	p_Evt->m_eventFunc = threadFunc;
	// increment callback count
	mp_workerObj->incrementCallbackCount();
	// post event to thread
	QCoreApplication::postEvent(mp_workerObj, p_Evt, priority);
	// success
	return true;
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
	if (!mp_workerThread->isRunning() || m_requestedQuit)
	{
		return -1;
	}
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
	}, Qt::HighEventPriority);
	// return loop id (we still do not have timerId)
	return newLoopId;
}

QDefer QLambdaThreadWorkerData::stopLoopInThread(const int &intLoopId)
{
	QDefer retDefer;
	if (!mp_workerThread->isRunning() || m_requestedQuit)
	{
		retDefer.reject();
		return retDefer;
	}
	// create function to stop loop, serialize map access by using event queue
	this->execInThread([this, intLoopId, retDefer]() mutable {
		if (!m_mapIdtimerIds.contains(intLoopId))
		{
			qWarning() << "QLambdaThreadWorker::stopLoopInThread : Invalid loop Id.";
			retDefer.reject();
			return;
		}
		// get real timer id of timer to stop
		int timerId = m_mapIdtimerIds.take(intLoopId);
		// remove function from map
		mp_workerObj->m_mapFuncs.remove(timerId);
		// stop timer
		mp_workerObj->killTimer(timerId);
		// success
		retDefer.resolve();
	}, Qt::HighEventPriority);
	// return promise
	return retDefer;
}

QDefer QLambdaThreadWorkerData::stopAllLoopsInThread()
{
	QDefer retDefer;
	if (!mp_workerThread->isRunning() || m_requestedQuit)
	{
		retDefer.reject();
		return retDefer;
	}
	// create function to stop loop, serialize map access by using event queue
	this->execInThread([this, retDefer]() mutable {
		while (m_mapIdtimerIds.count() > 0)
		{
			// get real timer id of timer to stop
			int timerId = m_mapIdtimerIds.take(m_mapIdtimerIds.first());
			// remove function from map
			mp_workerObj->m_mapFuncs.remove(timerId);
			// stop timer
			mp_workerObj->killTimer(timerId);
		}
		// success
		retDefer.resolve();
	}, Qt::HighEventPriority);
	// return promise
	return retDefer;
}

bool QLambdaThreadWorkerData::moveQObjectToThread(QObject * pObject)
{
	if (!mp_workerThread->isRunning() || m_requestedQuit)
	{
		return false;
	}
	pObject->moveToThread(mp_workerThread);
	// success
	return true;
}

QDefer QLambdaThreadWorkerData::quitThread()
{
	QDefer retDefer;
	if (mp_workerThread->isFinished() || m_requestedQuit)
	{
		retDefer.resolve();
		return retDefer;
	}
	// subscribe to thread finished
	QObject::connect(mp_workerThread, &QThread::finished, mp_workerObj, [retDefer]() mutable {
		retDefer.resolve();
	});
	// stop all loops
	auto defLoops = this->stopAllLoopsInThread();
	// stop accepting new callbacks or loops
	m_requestedQuit = true;
	// wait for loops stopped
	defLoops.done([this, retDefer]() {
	// wait for remaining callbacks to be processed
	QObject::connect(mp_workerObj, &QLambdaThreadWorkerObjectData::finishedProcessingCallbacks, mp_workerObj,
		[this, retDefer]() mutable {
			// actually quit thread
			mp_workerThread->quit();
		});
	}, Qt::DirectConnection);
	// return promise
	return retDefer;
}
