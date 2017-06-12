#ifndef QDYNAMICEVENTS_H
#define QDYNAMICEVENTS_H

#include <QCoreApplication>
#include <QObject>
#include <QList>
#include <QThread>
#include <QMutex>
#include <QMap>
#include <functional>

#define QDEFERREDPROXY_EVENT_TYPE (QEvent::Type)(QEvent::User + 666)

class QDynamicEventsProxyObject : public QObject
{
	Q_OBJECT
public:
	explicit QDynamicEventsProxyObject();

	bool event(QEvent* ev);

};

class QDynamicEventsProxyEvent : public QEvent
{
public:
	explicit QDynamicEventsProxyEvent();

	std::function<void()> m_eventFunc;

};

// have all template classes derive from common base class which to contains the static members
class QDynamicEventsBase {

protected:

	// make friend, so it can access whenInternal methods
	template<class ...Types>
	friend class QDynamicEvents;

	static QDynamicEventsProxyObject * getObjectForThread(QThread * p_currThd);

	static QMap< QThread *, QDynamicEventsProxyObject * > s_threadMap;

	static QMutex s_mutex;

};

#endif // QDYNAMICEVENTS_H