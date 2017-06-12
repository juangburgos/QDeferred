#ifndef QDYNAMICEVENTS_H
#define QDYNAMICEVENTS_H

#include <QCoreApplication>
#include <QObject>
#include <QList>
#include <QThread>
#include <QMutex>
#include <QMap>
#include <functional>

#define QDYNAMICEVENTSPROXY_EVENT_TYPE (QEvent::Type)(QEvent::User + 666)

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



	static QDynamicEventsProxyObject * getObjectForThread(QThread * p_currThd);

	static QMap< QThread *, QDynamicEventsProxyObject * > s_threadMap;

	static QMutex s_mutex;

};

// forward declaration to be able to make friend
template<class ...Types>
class QDynamicEvents;
// create a handle class to be able to remove event subscription
class QDynamicEventsHandle
{
	QDynamicEventsHandle(QString strEventName, QThread * p_handleThread, size_t funcId);
private:
	// make friend, so it can access internal methods
	template<class ...Types>
	friend class QDynamicEvents;

	QString   m_strEventName ;
	QThread * mp_handleThread;
	size_t    m_funcId;
};


// the actual dynamic events object implementation, Types are the callback arguments
template<class ...Types>
class QDynamicEvents : public QDynamicEventsBase
{
public:
	~QDynamicEvents();

	// consumer API

	// on method	
	QDynamicEventsHandle on(QString strEventName, std::function<void(Types(&...args))> callback);

	// provider API
	void trigger(QString strEventName, Types(&...args));

private:
	// thread safety first
	QMutex m_mutex;
	// list of connections to avoid memory leaks
	QList<QMetaObject::Connection> m_connectionList;
	// map of maps of maps, multiple callbacks by:
	QMap< QString, // by event name
		QMap< QThread *, // by thread
			QMap< size_t, // a hash or identifier for the function
					std::function<void(Types(&...args))>
				> 
			> 
		> m_callbacksMap;
	// create proxy object for unknown thread
	void createProxyObj(QString &strEventName);
	// internal on
	void onInternal(QString &strEventName, std::function<void(Types(&...args))> callback);
};

// alias for no argument types
using QDynEvts = QDynamicEvents<>;

template<class ...Types>
QDynamicEvents<Types...>::~QDynamicEvents()
{
	// remove connections
	for (int i = 0; i < m_connectionList.count(); i++)
	{
		QObject::disconnect(m_connectionList[i]);
	}
	m_callbacksMap.clear();
}

template<class ...Types>
void QDynamicEvents<Types...>::createProxyObj(QString &strEventName)
{
	QMutexLocker locker(&m_mutex);
	// get current thread
	QThread * p_currThd = QThread::currentThread();
	// if not in list then...
	if (!m_callbacksMap[strEventName].contains(p_currThd))
	{
		QDynamicEventsProxyObject * p_obj = QDynamicEventsBase::getObjectForThread(p_currThd);
		// wait until object destroyed to remove callbacks struct
		// NOTE : need to disconnect these connections to avoid memory leaks due to lambda memory allocations
		m_connectionList.append(QObject::connect(p_obj, &QObject::destroyed, [&, p_currThd]() {
			// delete callbacks when thread gets deleted
			m_callbacksMap[strEventName].take(p_currThd);
		}));
	};
}

template<class ...Types>
QDynamicEventsHandle QDynamicEvents<Types...>::on(QString strEventName, std::function<void(Types(&...args))> callback)
{
	// split by spaces
	QStringList listEventNames = strEventName.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	// create proxy object if necessary
	this->createProxyObj(strEventName);
	// lock after
	QMutexLocker locker(&m_mutex);
	// for each event name
	for (int i = 0; i < listEventNames.count(); i++)
	{
		onInternal(listEventNames[i], callback);
	}
	// return hash
	return QDynamicEventsHandle(strEventName, QThread::currentThread(), (size_t)(&callback));
}

template<class ...Types>
void QDynamicEvents<Types...>::onInternal(QString &strEventName, std::function<void(Types(&...args))> callback)
{
	m_callbacksMap[strEventName][QThread::currentThread()][(size_t)(&callback)] = callback;
}

template<class ...Types>
void QDynamicEvents<Types...>::trigger(QString strEventName, Types(&...args))
{
	QMutexLocker locker(&m_mutex);
	// for each thread where there are callbacks to be called
	auto listThreads = m_callbacksMap[strEventName].keys();
	for (int i = 0; i < listThreads.count(); i++)
	{
		auto p_currThread = listThreads.at(i);
		auto p_currObject = QDynamicEventsBase::getObjectForThread(p_currThread);
		// create object in heap and assign function (event loop takes ownership and deletes it later)
		QDynamicEventsProxyEvent * p_Evt = new QDynamicEventsProxyEvent;
		p_Evt->m_eventFunc = [this, strEventName, p_currThread, args...]() mutable {
			auto listHandles = this->m_callbacksMap[strEventName][p_currThread].keys();
			for (int j = 0; j < listHandles.count(); j++)
			{
				auto currHandle = listHandles.at(j);
				this->m_callbacksMap[strEventName][p_currThread][currHandle](args...);
			}			
		};
		// post event for object with correct thread affinity
		QCoreApplication::postEvent(p_currObject, p_Evt);
	}
}

#endif // QDYNAMICEVENTS_H