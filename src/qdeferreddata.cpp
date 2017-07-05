#include "qdeferreddata.hpp"

#include <QDebug>

QDeferredProxyObject::QDeferredProxyObject() : QObject(nullptr)
{
	// nothing to do here
}

bool QDeferredProxyObject::event(QEvent * ev)
{
	if (ev->type() == QDEFERREDPROXY_EVENT_TYPE) {
		// call function
		static_cast<QDeferredProxyEvent*>(ev)->m_eventFunc();
		// return event processed
		return true;
	}
	// Call base implementation (make sure the rest of events are handled)
	return QObject::event(ev);
}

QDeferredProxyEvent::QDeferredProxyEvent() : QEvent(QDEFERREDPROXY_EVENT_TYPE)
{
	// nothing to do here
}

// define static members and methods of base class
QMutex QDeferredDataBase::s_mutex;
QMap< QThread *, QDeferredProxyObject * > QDeferredDataBase::s_threadMap;
// TODO : use object below to clean static resource gracefully on exit
QObject QDeferredDataBase::s_objExitCleaner;

QDeferredProxyObject * QDeferredDataBase::getObjectForThread(QThread * p_currThd)
{
	static bool isFirstTime = true;
	if (isFirstTime)
	{
		QObject::connect(&QDeferredDataBase::s_objExitCleaner, &QObject::destroyed, []() {
			// TODO : cleanup possible connections from QThread::finished events below (used to crash)
			qDebug() << "QDeferredDataBase was deleted.";
		});
		isFirstTime = false;
	}
	// lock multithread access
	QMutexLocker locker(&QDeferredDataBase::s_mutex);
	// if not in list then...
	if (!QDeferredDataBase::s_threadMap.contains(p_currThd))
	{
		// subscribe to finish
		QObject::connect(p_currThd, &QThread::finished, [p_currThd]() {
			// if finished, remove
			auto p_objToDelete = QDeferredDataBase::s_threadMap.take(p_currThd);
			// mark the object for deletion
			p_objToDelete->deleteLater();
		});
		// add to all maps
		QDeferredDataBase::s_threadMap[p_currThd] = new QDeferredProxyObject;
	}
	// return
	return QDeferredDataBase::s_threadMap[p_currThd];
}

