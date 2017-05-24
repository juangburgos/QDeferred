#include "qdeferredproxyobject.h"

QDeferredProxyObject::QDeferredProxyObject() : QObject(nullptr)
{
	// nothing to do here
}


QDeferredProxyObject::QDeferredProxyObject(const QDeferredProxyObject &obj) : m_perThreadFunc(obj.m_perThreadFunc)
{
	// TODO : this changes the affinity due to QMap, try POINTER
}

QDeferredProxyObject & QDeferredProxyObject::operator=(const QDeferredProxyObject &rhs)
{

	// TODO : this changes the affinity due to QMap, try POINTER

	m_perThreadFunc = rhs.m_perThreadFunc;
	return *this;
}

#include <QDebug>
#include <QThread>
bool QDeferredProxyObject::event(QEvent * ev)
{
	if (ev->type() == QDEFERREDPROXY_EVENT_TYPE) {
		qDebug() << "[INFO] Event for object thread affinity " << this->thread() << " from thread " << QThread::currentThread();
		// call function
		this->m_perThreadFunc();	
		// return event processed
		return true;
	}
	// Call base implementation (make sure the rest of events are handled)
	return QObject::event(ev);
}

QDeferredProxyEvent::QDeferredProxyEvent() : QEvent(QDEFERREDPROXY_EVENT_TYPE)
{

}
