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

}

// define static members and methods of base class
QMutex QDeferredDataBase::s_mutex;
QMap< QThread *, QDeferredProxyObject * > QDeferredDataBase::s_threadMap;

QDeferredProxyObject * QDeferredDataBase::getObjectForThread(QThread * p_currThd)
{
	// lock multithread access
	QDeferredDataBase::s_mutex.lock();
	// if not in list then...
	if (!s_threadMap.contains(p_currThd))
	{
		// subscribe to finish
		QObject::connect(p_currThd, &QThread::finished, [p_currThd]() {
			// if finished, remove
			auto p_objToDelete = QDeferredDataBase::s_threadMap.take(p_currThd);
			// mark the object for deletion
			p_objToDelete->deleteLater();
		});
		// add to all maps
		s_threadMap[p_currThd] = new QDeferredProxyObject;
	}
	// unlock multithread access
	QDeferredDataBase::s_mutex.unlock();
	// return
	return s_threadMap[p_currThd];
}

