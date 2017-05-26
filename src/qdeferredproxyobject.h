#ifndef QDEFERREDPROXYOBJECT_H
#define QDEFERREDPROXYOBJECT_H

#include <QObject>
#include <QEvent>
#include <functional>

#define QDEFERREDPROXY_EVENT_TYPE (QEvent::Type)(QEvent::User + 123)

class QDeferredProxyObject : public QObject
{
    Q_OBJECT
public:
    explicit QDeferredProxyObject();

	bool event(QEvent* ev);

};

class QDeferredProxyEvent : public QEvent
{
public:
	explicit QDeferredProxyEvent();

	std::function<void()> m_eventFunc;

};

#endif // QDEFERREDPROXYOBJECT_H