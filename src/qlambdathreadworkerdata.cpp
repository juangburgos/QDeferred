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
	// get thread id
	std::stringstream stream;
	stream << std::hex << (size_t)mp_workerThread;
	m_strThreadId = "QThread(0x" + QString::fromStdString(stream.str()) + ")";
	// move worker object to thread and subscribe for deletion
	mp_workerObj->moveToThread(mp_workerThread);
	QObject::connect(mp_workerThread, &QThread::finished, mp_workerObj, &QObject::deleteLater);
	// start thread
	mp_workerThread->start();
}

QLambdaThreadWorkerData::QLambdaThreadWorkerData(const QLambdaThreadWorkerData &other) : QSharedData(other),
mp_workerObj(other.mp_workerObj)
{
	this->mp_workerThread = other.mp_workerThread; 
}

QLambdaThreadWorkerData::~QLambdaThreadWorkerData()
{
	mp_workerThread->quit();
	mp_workerThread->wait();
	mp_workerThread->deleteLater();
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
