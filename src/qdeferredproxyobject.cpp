#include "qdeferredproxyobject.h"

//// DEBUG
//#include <QDebug>
//#include <QThread>

QDeferredProxyObject::QDeferredProxyObject() : QObject(nullptr)
{
	
}

bool QDeferredProxyObject::event(QEvent * ev)
{
	if (ev->type() == QDEFERREDPROXY_EVENT_TYPE) {
		//// DEBUG
		//qDebug() << "[INFO] Event for object thread affinity " << this->thread() << " from thread " << QThread::currentThread();
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
