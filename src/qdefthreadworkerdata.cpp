#include "qdefthreadworkerdata.h"
#include <QCoreApplication>
#include <sstream>

QDefThreadWorkerDataEvent::QDefThreadWorkerDataEvent() : QEvent(QDEFTHREADWORKERDATA_EVENT_TYPE)
{
	// nothing to do here
}

QDefThreadWorkerObjectData::QDefThreadWorkerObjectData() : QObject(nullptr)
{
	// nothing to do here either
}

bool QDefThreadWorkerObjectData::event(QEvent * ev)
{
	if (ev->type() == QDEFTHREADWORKERDATA_EVENT_TYPE) {
		// call function
		static_cast<QDefThreadWorkerDataEvent*>(ev)->m_eventFunc();
		// return event processed
		return true;
	}
	// Call base implementation (make sure the rest of events are handled)
	return QObject::event(ev);
}

QDefThreadWorkerData::QDefThreadWorkerData()
{
	mp_workerThread = new QThread;
	mp_workerObj    = new QDefThreadWorkerObjectData;
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

QDefThreadWorkerData::QDefThreadWorkerData(const QDefThreadWorkerData &other) : QSharedData(other),
mp_workerObj(other.mp_workerObj)
{
	this->mp_workerThread = other.mp_workerThread; 
}

QDefThreadWorkerData::~QDefThreadWorkerData()
{
	mp_workerThread->quit();
	mp_workerThread->wait();
	mp_workerThread->deleteLater();
}

void QDefThreadWorkerData::execInThread(std::function<void()> &threadFunc)
{
	// create event to exec in thread
	QDefThreadWorkerDataEvent * p_Evt = new QDefThreadWorkerDataEvent;
	p_Evt->m_eventFunc = threadFunc;
	// post event to thread
	QCoreApplication::postEvent(mp_workerObj, p_Evt);
}

QString QDefThreadWorkerData::getThreadId()
{
	return m_strThreadId;
}
