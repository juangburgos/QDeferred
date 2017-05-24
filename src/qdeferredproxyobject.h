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
	QDeferredProxyObject(const QDeferredProxyObject &obj);
	QDeferredProxyObject &operator=(const QDeferredProxyObject &);

	bool event(QEvent* ev);

	std::function<void()> m_perThreadFunc;

};

class QDeferredProxyEvent : public QEvent
{
public:
	explicit QDeferredProxyEvent();

};

#endif // QDEFERREDPROXYOBJECT_H