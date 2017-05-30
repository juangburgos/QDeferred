#include "qdeferredproxyobject.h"

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
