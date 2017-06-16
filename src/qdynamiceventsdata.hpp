#ifndef QDYNAMICEVENTSDATA_H
#define QDYNAMICEVENTSDATA_H

#include <QCoreApplication>
#include <QSharedData>
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
class QDynamicEventsDataBase {

protected:
	static QDynamicEventsProxyObject * getObjectForThread(QThread * p_currThd);
	static QMap< QThread *, QDynamicEventsProxyObject * > s_threadMap;
	static QMutex    s_mutex;
	static qlonglong s_funcId;
};

// forward declaration to be able to make friend
template<class ...Types>
class QDynamicEventsData;
// create a handle class to be able to remove event subscription
class QDynamicEventsHandle
{
	QDynamicEventsHandle(QString strEventName, QThread * p_handleThread, qlonglong funcId);
private:
	// make friend, so it can access internal methods
	template<class ...Types>
	friend class QDynamicEventsData;

	QString   m_strEventName ;
	QThread * mp_handleThread;
	qlonglong m_funcId;
};

// forward declaration to be able to pass as arg
template<class ...Types>
class QDynamicEvents;

// the actual dynamic events object implementation, Types are the callback arguments
template<class ...Types>
class QDynamicEventsData : public QSharedData, public QDynamicEventsDataBase
{
public:
	QDynamicEventsData();
	QDynamicEventsData(const QDynamicEventsData &other);
	~QDynamicEventsData();

	// consumer API

	// on method	
	QDynamicEventsHandle on(QString strEventName, std::function<void(Types(&...args))> callback);
	// once method	
	QDynamicEventsHandle once(QString strEventName, std::function<void(Types(&...args))> callback);
	// off method (all callbacks registered to an specific event name)
	void off(QString strEventName);
	// off method (specific callback based on handle)
	void off(QDynamicEventsHandle evtHandle);
	// off method (all callbacks)
	void off();

	// provider API

	void trigger(QDynamicEvents<Types...> ref, QString strEventName, Types(&...args));

private:
	// thread safety first
	QMutex m_mutex;
	// list of connections to avoid memory leaks
	QList<QMetaObject::Connection> m_connectionList;
	// map of maps of maps, multiple callbacks by:
	QMap< QString,        // by event name
		QMap< QThread *,  // by thread
			QMap< qlonglong, // an identifier for the function
					std::function<void(Types(&...args))>
				> 
			> 
		> m_callbacksMap;
	// map of maps of maps, multiple callbacks by:
	QMap< QString,        // by event name
		QMap< QThread *,  // by thread
			QMap< qlonglong, // an identifier for the function
					std::function<void(Types(&...args))>
				>
			>
		> m_callbacksMapOnce;
	// create proxy object for unknown thread
	void createProxyObj(QString &strEventName);
	// internal on
	void onInternal(QString &strEventName, std::function<void(Types(&...args))> callback, qlonglong &funcId);
	// internal once
	void onceInternal(QString &strEventName, std::function<void(Types(&...args))> callback, qlonglong &funcId);
	// off method (all callbacks registered to an specific event name)
	void offInternal(QString &strEventName);
	// off method (specific callback based on handle)
	void offInternal(QString &strEventName, QThread *pThread, qlonglong &funcId);
	// internal trigger
	void triggerInternal(QDynamicEvents<Types...> ref, QString &strEventName, Types(&...args));
};

template<class ...Types>
QDynamicEventsData<Types...>::QDynamicEventsData()
{
	// nothing to do here
}

template<class ...Types>
QDynamicEventsData<Types...>::QDynamicEventsData(const QDynamicEventsData &other) : QSharedData(other),
m_mutex(other.m_mutex),
m_connectionList(other.m_connectionList),
m_callbacksMap(other.m_callbacksMap)
{
	// nothing to do here
}

template<class ...Types>
QDynamicEventsData<Types...>::~QDynamicEventsData()
{
	//QMutexLocker locker(&m_mutex);
	// remove connections
	for (int i = 0; i < m_connectionList.count(); i++)
	{
		QObject::disconnect(m_connectionList[i]);
	}
	m_callbacksMap.clear();
	m_callbacksMapOnce.clear();
}

template<class ...Types>
void QDynamicEventsData<Types...>::off(QString strEventName)
{
	QMutexLocker locker(&m_mutex);
	// split by spaces
	QStringList listEventNames = strEventName.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	// for each event name
	for (int i = 0; i < listEventNames.count(); i++)
	{
		offInternal(listEventNames[i]);
	}
}

template<class ...Types>
void QDynamicEventsData<Types...>::offInternal(QString &strEventName)
{
	// remove for all threads and all callbacks
	m_callbacksMap.remove(strEventName);
	m_callbacksMapOnce.remove(strEventName);
}

template<class ...Types>
void QDynamicEventsData<Types...>::off(QDynamicEventsHandle evtHandle)
{
	QMutexLocker locker(&m_mutex);
	// split by spaces
	QStringList listEventNames = evtHandle.m_strEventName.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	// for each event name
	for (int i = 0; i < listEventNames.count(); i++)
	{
		offInternal(listEventNames[i], evtHandle.mp_handleThread, evtHandle.m_funcId);
	}
}

template<class ...Types>
void QDynamicEventsData<Types...>::offInternal(QString &strEventName, QThread *pThread, qlonglong &funcId)
{
	// remove very specific callback
	m_callbacksMap[strEventName][pThread].remove(funcId);
	m_callbacksMapOnce[strEventName][pThread].remove(funcId);
}

template<class ...Types>
void QDynamicEventsData<Types...>::off()
{
	QMutexLocker locker(&m_mutex);
	// remove all callbacks
	m_callbacksMap.clear();
	m_callbacksMapOnce.clear();
}

template<class ...Types>
void QDynamicEventsData<Types...>::createProxyObj(QString &strEventName)
{
	QMutexLocker locker(&m_mutex);
	// get current thread
	QThread * p_currThd = QThread::currentThread();
	// create obj for thread if not existing, else return existing
	QDynamicEventsProxyObject * p_obj = QDynamicEventsDataBase::getObjectForThread(p_currThd);
	// split by spaces
	QStringList listEventNames = strEventName.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	// for each event name
	for (int i = 0; i < listEventNames.count(); i++)
	{
		QString strCurrEvtName = listEventNames[i];
		// wait until object destroyed to remove callbacks struct
		// NOTE : need to disconnect these connections to avoid memory leaks due to lambda memory allocations
		m_connectionList.append(QObject::connect(p_obj, &QObject::destroyed, [this, strCurrEvtName, p_currThd]() {
			// delete callbacks when thread gets deleted
			this->m_callbacksMap[strCurrEvtName].remove(p_currThd);
			this->m_callbacksMapOnce[strCurrEvtName].remove(p_currThd);
		}));
	}
}

template<class ...Types>
QDynamicEventsHandle QDynamicEventsData<Types...>::on(QString strEventName, std::function<void(Types(&...args))> callback)
{
	// split by spaces
	QStringList listEventNames = strEventName.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	// create proxy object if necessary
	this->createProxyObj(strEventName);
	// get callback uuid
	qlonglong funcId = QDynamicEventsDataBase::s_funcId++;
	// lock after
	QMutexLocker locker(&m_mutex);
	// for each event name
	for (int i = 0; i < listEventNames.count(); i++)
	{
		onInternal(listEventNames[i], callback, funcId);
	}
	// return hash
	return QDynamicEventsHandle(strEventName, QThread::currentThread(), funcId);
}

template<class ...Types>
void QDynamicEventsData<Types...>::onInternal(QString &strEventName, std::function<void(Types(&...args))> callback, qlonglong &funcId)
{
	// [NOTE] No lock in internal methods
	m_callbacksMap[strEventName][QThread::currentThread()][funcId] = callback;
}

template<class ...Types>
QDynamicEventsHandle QDynamicEventsData<Types...>::once(QString strEventName, std::function<void(Types(&...args))> callback)
{
	// split by spaces
	QStringList listEventNames = strEventName.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	// create proxy object if necessary
	this->createProxyObj(strEventName);
	// get callback uuid
	qlonglong funcId = QDynamicEventsDataBase::s_funcId++;
	// lock after
	QMutexLocker locker(&m_mutex);
	// for each event name
	for (int i = 0; i < listEventNames.count(); i++)
	{
		onceInternal(listEventNames[i], callback, funcId);
	}
	// return hash
	return QDynamicEventsHandle(strEventName, QThread::currentThread(), funcId);
}

template<class ...Types>
void QDynamicEventsData<Types...>::onceInternal(QString &strEventName, std::function<void(Types(&...args))> callback, qlonglong &funcId)
{
	// [NOTE] No lock in internal methods
	m_callbacksMapOnce[strEventName][QThread::currentThread()][funcId] = callback;
}

template<class ...Types>
void QDynamicEventsData<Types...>::trigger(QDynamicEvents<Types...> ref, QString strEventName, Types(&...args))
{
	QMutexLocker locker(&m_mutex);	
	// split by spaces
	QStringList listEventNames = strEventName.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	// for each event name
	for (int i = 0; i < listEventNames.count(); i++)
	{
		triggerInternal(ref, listEventNames[i], args...);
	}
}

template<class ...Types>
void QDynamicEventsData<Types...>::triggerInternal(QDynamicEvents<Types...> ref, QString &strEventName, Types(&...args))
{
	// [NOTE] No lock in internal methods

	// on method callbacks *********************************************************
	// for each thread where there are callbacks to be called
	auto listThreads = m_callbacksMap[strEventName].keys();
	for (int i = 0; i < listThreads.count(); i++)
	{
		auto p_currThread = listThreads.at(i);
		auto p_currObject = QDynamicEventsDataBase::getObjectForThread(p_currThread);
		// create object in heap and assign function (event loop takes ownership and deletes it later)
		QDynamicEventsProxyEvent * p_Evt = new QDynamicEventsProxyEvent;
		// NOTE need to pass mapOnlyCallbacks as copy because if an off() gets execd before event loop resumes, callbacks will not be called
		auto mapOnlyCallbacks = this->m_callbacksMap[strEventName][p_currThread];
		p_Evt->m_eventFunc = [ref, mapOnlyCallbacks, args...]() mutable {
			auto listHandles = mapOnlyCallbacks.keys();
			for (int j = 0; j < listHandles.count(); j++)
			{
				auto currHandle = listHandles.at(j);
				mapOnlyCallbacks[currHandle](args...);
			}
			// unused, but we need it to keep at least one reference until all callbacks are executed
			Q_UNUSED(ref)
		};
		// post event for object with correct thread affinity
		QCoreApplication::postEvent(p_currObject, p_Evt);
	}

	// once method callbacks *********************************************************
	// for each thread where there are callbacks to be called
	auto listThreadsOnce = m_callbacksMapOnce[strEventName].keys();
	for (int i = 0; i < listThreadsOnce.count(); i++)
	{
		auto p_currThread = listThreadsOnce.at(i);
		auto p_currObject = QDynamicEventsDataBase::getObjectForThread(p_currThread);
		// create object in heap and assign function (event loop takes ownership and deletes it later)
		QDynamicEventsProxyEvent * p_Evt = new QDynamicEventsProxyEvent;
		// NOTE below the difference is the 'take' method which ensures callbacks are only execd once
		auto mapOnlyCallbacksOnce = this->m_callbacksMapOnce[strEventName].take(p_currThread);
		p_Evt->m_eventFunc = [ref, mapOnlyCallbacksOnce, args...]() mutable {
			auto listHandles = mapOnlyCallbacksOnce.keys();
			for (int j = 0; j < listHandles.count(); j++)
			{
				auto currHandle = listHandles.at(j);
				mapOnlyCallbacksOnce[currHandle](args...);
			}
			// unused, but we need it to keep at least one reference until all callbacks are executed
			Q_UNUSED(ref)
		};
		// post event for object with correct thread affinity
		QCoreApplication::postEvent(p_currObject, p_Evt);
	}
}

#endif // QDYNAMICEVENTSDATA_H