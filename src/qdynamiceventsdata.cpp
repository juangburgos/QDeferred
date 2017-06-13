#include "qdynamicevents.hpp"

QDynamicEventsProxyObject::QDynamicEventsProxyObject() : QObject(nullptr)
{
	// nothing to do here
}

bool QDynamicEventsProxyObject::event(QEvent * ev)
{
	if (ev->type() == QDYNAMICEVENTSPROXY_EVENT_TYPE) {
		// call function
		static_cast<QDynamicEventsProxyEvent*>(ev)->m_eventFunc();
		// return event processed
		return true;
	}
	// Call base implementation (make sure the rest of events are handled)
	return QObject::event(ev);
}

QDynamicEventsProxyEvent::QDynamicEventsProxyEvent() : QEvent(QDYNAMICEVENTSPROXY_EVENT_TYPE)
{
	// nothing to do here
}

// define static members and methods of base class
QMutex QDynamicEventsDataBase::s_mutex;
QMap< QThread *, QDynamicEventsProxyObject * > QDynamicEventsDataBase::s_threadMap;

QDynamicEventsProxyObject * QDynamicEventsDataBase::getObjectForThread(QThread * p_currThd)
{
	// lock multithread access
	QMutexLocker locker(&QDynamicEventsDataBase::s_mutex);
	// if not in list then...
	if (!s_threadMap.contains(p_currThd))
	{
		// subscribe to finish
		QObject::connect(p_currThd, &QThread::finished, [p_currThd]() {
			// if finished, remove
			auto p_objToDelete = QDynamicEventsDataBase::s_threadMap.take(p_currThd);
			// mark the object for deletion
			p_objToDelete->deleteLater();
		});
		// add to all maps
		s_threadMap[p_currThd] = new QDynamicEventsProxyObject;
	}
	// return
	return s_threadMap[p_currThd];
}

QDynamicEventsHandle::QDynamicEventsHandle(QString strEventName, QThread * p_handleThread, size_t funcId)
{
	m_strEventName  = strEventName;
	mp_handleThread = p_handleThread;
	m_funcId        = funcId;
}
