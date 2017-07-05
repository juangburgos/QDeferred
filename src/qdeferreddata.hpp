#ifndef QDEFERREDDATA_H
#define QDEFERREDDATA_H

#include <QCoreApplication>
#include <QSharedData>
#include <QList>
#include <QThread>
#include <QMutex>
#include <QMap>
#include <functional>
#include <QObject>
#include <QEvent>

// custom event to be used in qt event loop for each thread
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

// have all template classes derive from common base class which to contains the static members
// forward declaration to be able to make friend
template<class ...Types>
class QDeferred;
// base class
class QDeferredDataBase {

protected:

	// make friend, so it can access whenInternal methods
	template<class ...Types>
	friend class QDeferred;

	// NOTE : recursive unpacking of parameter pack
	// http://kevinushey.github.io/blog/2016/01/27/introduction-to-c++-variadic-templates/
	template<class T>
	static void whenInternal(std::function<void()> doneCallback, std::function<void()> failCallback, T t)
	{
		// add to done zero params list
		t.doneZero(doneCallback);
		// add to fail zero params list
		t.failZero(failCallback);
	}

	template<class T, class... Rest>
	static void whenInternal(std::function<void()> doneCallback, std::function<void()> failCallback, T t, Rest... rest)
	{
		// process single deferred
		whenInternal(doneCallback, failCallback, t);
		// expand by recursion, process rest of deferreds
		whenInternal(doneCallback, failCallback, rest...);
	}

	static QObject s_objExitCleaner;

	static QDeferredProxyObject * getObjectForThread(QThread * p_currThd);

	static QMap< QThread *, QDeferredProxyObject * > s_threadMap;

	static QMutex s_mutex;

};

enum QDeferredState
{
	PENDING,
	RESOLVED,
	REJECTED
};

// the actual deferred object implementation, Types are the callback arguments
template<class ...Types>
class QDeferredData : public QSharedData, public QDeferredDataBase
{
public:
	// constructors
	QDeferredData();
	QDeferredData(const QDeferredData &other);
	~QDeferredData();

	// consumer API

	// get state method
	QDeferredState state();

	// done method	
	void done(std::function<void(Types (&...args))> callback); // by copy would be <Types... arg>
	// fail method
	void fail(std::function<void(Types(&...args))> callback);
	// then method
	void then(std::function<void(Types(&...args))> callback);
	// progress method
	void progress(std::function<void(Types(&...args))> callback);

	// consumer visual studio debug API (native visualizations)
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
	// debug done method	
	void doneVsDbg_Impl(QString &strVsDbg, std::function<void(Types(&...args))> callback);
	// debug fail method
	void failVsDbg_Impl(QString &strVsDbg, std::function<void(Types(&...args))> callback);
	// debug then method
	void thenVsDbg_Impl(QString &strVsDbg, std::function<void(Types(&...args))> callback);
	// debug progress method
	void progressVsDbg_Impl(QString &strVsDbg, std::function<void(Types(&...args))> callback);
#endif

	// provider API

	// resolve method (ref added to avoid last reference deletion before callbacks execution)
	void resolve(QDeferred<Types...> ref, Types(&...args));
	// reject method
	void reject(QDeferred<Types...> ref, Types(&...args));
	// notify method
	void notify(QDeferred<Types...> ref, Types(&...args));

	// provider visual studio debug API (native visualizations)
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
	// debug resolve method
	void resolveVsDbg_Impl(QString &strVsDbg, QDeferred<Types...> ref, Types(&...args));
	// debug reject method
	void rejectVsDbg_Impl(QString &strVsDbg, QDeferred<Types...> ref, Types(&...args));
	// debug notify method
	void notifyVsDbg_Impl(QString &strVsDbg, QDeferred<Types...> ref, Types(&...args));
#endif

	// internal API

	// done method with zero arguments
	void doneZero(std::function<void()> callback);
	// fail method with zero arguments
	void failZero(std::function<void()> callback);

	// when memory
	int m_whenCount = 0;

private:
	// structure to contain callbacks (one instance per thread in m_callbacksMap)
	struct DeferredAllCallbacks {
		QList< std::function<void(Types(&...args))> > m_doneList;
		QList< std::function<void(Types(&...args))> > m_failList;
		QList< std::function<void(Types(&...args))> > m_thenList;
		QList< std::function<void(Types(&...args))> > m_progressList;
		QList< std::function<void()               > > m_doneZeroList;
		QList< std::function<void()               > > m_failZeroList;
	};
	// members
	std::function<void(std::function<void(Types(&...args))>)> m_finishedFunction;
	QMap< QThread *, DeferredAllCallbacks * > m_callbacksMap;
	QDeferredState m_state;
	QMutex         m_mutex;
	QList<QMetaObject::Connection> m_connectionList;
	// methods
	void execute    (QList< std::function<void(Types(&...args))> > &listCallbacks, Types(&...args));
	void executeZero(QList< std::function<void()               > > &listCallbacks);
	DeferredAllCallbacks * getCallbaksForThread();

	// debug helpers for visual studio debug API (native visualizations)
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
	std::string d_state;
	std::vector<std::string> d_vecDoneList;
	std::vector<std::string> d_vecFailList;
	std::vector<std::string> d_vecThenList;
	std::vector<std::string> d_vecProgressList;
	std::vector<std::string> d_vecResolveList;
	std::vector<std::string> d_vecRejectList;
	std::vector<std::string> d_vecNotifyList;
#endif
};

template<class ...Types>
QDeferredData<Types...>::QDeferredData()
{
	m_state = QDeferredState::PENDING;
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
	d_state = "PENDING";
#endif
}

template<class ...Types>
QDeferredData<Types...>::~QDeferredData()
{
	// delete all memory allocated on heap
	qDeleteAll(m_callbacksMap);
	// remove connections
	for (int i = 0; i < m_connectionList.count(); i++)
	{
		QObject::disconnect(m_connectionList[i]);
	}
	m_callbacksMap.clear();
}

template<class ...Types>
QDeferredData<Types...>::QDeferredData(const QDeferredData &other) : QSharedData(other),
m_whenCount(other.m_whenCount),
m_callbacksMap(other.m_callbacksMap),
m_state(other.m_state),
m_mutex(other.m_mutex),
m_connectionList(other.m_connectionList),
m_finishedFunction(other.m_finishedFunction)
{
	// nothing to do here
}

template<class ...Types>
QDeferredState QDeferredData<Types...>::state()
{
	return m_state;
}

template<class ...Types>
void QDeferredData<Types...>::done(std::function<void(Types(&...args))> callback)
{
	// add object for thread if does not exists
	auto p_callbacks = this->getCallbaksForThread();
	// append to done callbacks list
	p_callbacks->m_doneList.append(callback);
	// call it inmediatly if already resolved
	QMutexLocker locker(&m_mutex);
	if (m_state == QDeferredState::RESOLVED)
	{
		m_finishedFunction(callback);
	}
}

template<class ...Types>
void QDeferredData<Types...>::fail(std::function<void(Types(&...args))> callback)
{
	// add object for thread if does not exists
	auto p_callbacks = this->getCallbaksForThread();
	// append to fail callbacks list
	p_callbacks->m_failList.append(callback);
	// call it inmediatly if already rejected
	QMutexLocker locker(&m_mutex);
	if (m_state == QDeferredState::REJECTED)
	{
		m_finishedFunction(callback);
	}
}

template<class ...Types>
void QDeferredData<Types...>::then(std::function<void(Types(&...args))> callback)
{
	// add object for thread if does not exists
	auto p_callbacks = this->getCallbaksForThread();
	// append to then callbacks list
	p_callbacks->m_thenList.append(callback);
	// call it inmediatly if already resolved or rejected
	QMutexLocker locker(&m_mutex);
	if (m_state == QDeferredState::RESOLVED || m_state == QDeferredState::REJECTED)
	{
		m_finishedFunction(callback);
	}
}

template<class ...Types>
void QDeferredData<Types...>::progress(std::function<void(Types(&...args))> callback)
{
	// add object for thread if does not exists
	auto p_callbacks = this->getCallbaksForThread();
	// append to then callbacks list
	p_callbacks->m_progressList.append(callback);
}

#if defined(QT_DEBUG) && defined(Q_OS_WIN)
template<class ...Types>
void QDeferredData<Types...>::doneVsDbg_Impl(QString &strVsDbg, std::function<void(Types(&...args))> callback)
{
	// store
	d_vecDoneList.push_back(strVsDbg.toStdString());
	// call original
	this->done(callback);
}

template<class ...Types>
void QDeferredData<Types...>::failVsDbg_Impl(QString &strVsDbg, std::function<void(Types(&...args))> callback)
{
	// store
	d_vecFailList.push_back(strVsDbg.toStdString());
	// call original
	this->fail(callback);
}

template<class ...Types>
void QDeferredData<Types...>::thenVsDbg_Impl(QString &strVsDbg, std::function<void(Types(&...args))> callback)
{
	// store
	d_vecThenList.push_back(strVsDbg.toStdString());
	// call original
	this->done(callback);
}

template<class ...Types>
void QDeferredData<Types...>::progressVsDbg_Impl(QString &strVsDbg, std::function<void(Types(&...args))> callback)
{
	// store
	d_vecProgressList.push_back(strVsDbg.toStdString());
	// call original
	this->progress(callback);
}
#endif // defined(QT_DEBUG) && defined(Q_OS_WIN)

template<class ...Types>
void QDeferredData<Types...>::resolve(QDeferred<Types...> ref, Types(&...args))
{
	QMutexLocker locker(&m_mutex);
	// early exit if deferred has been already resolved or rejected
	if (m_state != QDeferredState::PENDING)
	{
		qWarning() << "Cannot resolve already processed deferred object.";
		return;
	}
	// change state
	m_state = QDeferredState::RESOLVED;
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
	d_state = "RESOLVED";
#endif
	// set finished function (used to cache variadic args as a copy to be able to exec funcs added after resolve)
	m_finishedFunction = [args...](std::function<void(Types(&...args))> callback) mutable {
		callback(args...);
	};

	// for each thread where there are callbacks to be called
	QMapIterator< QThread *, DeferredAllCallbacks *> i(m_callbacksMap);
	while (i.hasNext()) 
	{
		i.next();
		// set test function
		auto p_currThread     = i.key();
		auto p_currObject     = QDeferredDataBase::getObjectForThread(p_currThread);
		auto p_currCallbacks  = m_callbacksMap[p_currThread];
		// create object in heap and assign function (event loop takes ownership and deletes it later)
		QDeferredProxyEvent * p_Evt = new QDeferredProxyEvent;
		p_Evt->m_eventFunc = [ref, this, p_currCallbacks, args...]() mutable {
			// execute all done callbacks
			this->execute(p_currCallbacks->m_doneList, args...); // DONE
			// execute all then callbacks
			this->execute(p_currCallbacks->m_thenList, args...); // THEN
			// loop all done zero callbacks
			this->executeZero(p_currCallbacks->m_doneZeroList);  // DONE
			// unused, but we need it to keep at least one reference until all callbacks are executed
			Q_UNUSED(ref)
		};
		// post event for object with correct thread affinity
		QCoreApplication::postEvent(p_currObject, p_Evt);
	}	
}

template<class ...Types>
void QDeferredData<Types...>::reject(QDeferred<Types...> ref, Types(&...args))
{
	QMutexLocker locker(&m_mutex);
	// early exit if deferred has been already resolved or rejected
	if (m_state != QDeferredState::PENDING)
	{
		qWarning() << "Cannot reject already processed deferred object.";
		return;
	}
	// change state
	m_state = QDeferredState::REJECTED;
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
	d_state = "REJECTED";
#endif
	// set finished function (used to cache variadic args as a copy to be able to exec funcs added after reject)
	m_finishedFunction = [args...](std::function<void(Types(&...args))> callback) mutable {
		callback(args...);
	};
	// for each thread where there are callbacks to be called
	QMapIterator< QThread *, DeferredAllCallbacks *> i(m_callbacksMap);
	while (i.hasNext())
	{
		i.next();
		// set test function
		auto p_currThread    = i.key();
		auto p_currObject    = QDeferredDataBase::getObjectForThread(p_currThread);
		auto p_currCallbacks = m_callbacksMap[p_currThread];
		// create object in heap and assign function (event loop takes ownership and deletes it later)
		QDeferredProxyEvent * p_Evt = new QDeferredProxyEvent;
		p_Evt->m_eventFunc = [ref, this, p_currCallbacks, args...]() mutable {
			// execute all done callbacks
			this->execute(p_currCallbacks->m_failList, args...); // FAIL
			// execute all then callbacks
			this->execute(p_currCallbacks->m_thenList, args...); // THEN
			// loop all done zero callbacks
			this->executeZero(p_currCallbacks->m_failZeroList);  // FAIL
			// unused, but we need it to keep at least one reference until all callbacks are executed
			Q_UNUSED(ref)
		};
		// post event for object with correct thread affinity
		QCoreApplication::postEvent(p_currObject, p_Evt);
	}
}

template<class ...Types>
void QDeferredData<Types...>::notify(QDeferred<Types...> ref, Types(&...args))
{
	QMutexLocker locker(&m_mutex);
	// early exit if deferred has been already resolved or rejected
	if (m_state != QDeferredState::PENDING)
	{
		qWarning() << "Cannot notify already processed deferred object.";
		return;
	}
	// for each thread where there are callbacks to be called
	QMapIterator< QThread *, DeferredAllCallbacks *> i(m_callbacksMap);
	while (i.hasNext())
	{
		i.next();
		// set test function
		auto p_currThread    = i.key();
		auto p_currObject    = QDeferredDataBase::getObjectForThread(p_currThread);
		auto p_currCallbacks = m_callbacksMap[p_currThread];
		// create object in heap and assign function (event loop takes ownership and deletes it later)
		QDeferredProxyEvent * p_Evt = new QDeferredProxyEvent;
		p_Evt->m_eventFunc = [ref, this, p_currCallbacks, args...]() mutable {
			// execute all done callbacks
			this->execute(p_currCallbacks->m_progressList, args...); // PROGRESS
			Q_UNUSED(ref)
		};
		// post event for object with correct thread affinity
		QCoreApplication::postEvent(p_currObject, p_Evt);
	}
}

#if defined(QT_DEBUG) && defined(Q_OS_WIN)
template<class ...Types>
void QDeferredData<Types...>::resolveVsDbg_Impl(QString &strVsDbg, QDeferred<Types...> ref, Types(&...args))
{
	// store
	d_vecResolveList.push_back(strVsDbg.toStdString());
	// call original
	this->resolve(ref, args...);
}

template<class ...Types>
void QDeferredData<Types...>::rejectVsDbg_Impl(QString &strVsDbg, QDeferred<Types...> ref, Types(&...args))
{
	// store
	d_vecRejectList.push_back(strVsDbg.toStdString());
	// call original
	this->reject(ref, args...);
}

template<class ...Types>
void QDeferredData<Types...>::notifyVsDbg_Impl(QString &strVsDbg, QDeferred<Types...> ref, Types(&...args))
{
	// store
	d_vecNotifyList.push_back(strVsDbg.toStdString());
	// call original
	this->notify(ref, args...);
}
#endif // defined(QT_DEBUG) && defined(Q_OS_WIN)

template<class ...Types>
void QDeferredData<Types...>::execute(QList< std::function<void(Types(&...args))> > &listCallbacks, Types(&...args))
{
	// loop all callbacks
	for (int i = 0; i < listCallbacks.length(); i++)
	{
		// call each callback with arguments
		listCallbacks.at(i)(args...);
	}
}

template<class ...Types>
void QDeferredData<Types...>::doneZero(std::function<void()> callback)
{
	// add object for thread if does not exists
	auto p_callbacks = this->getCallbaksForThread();
	// append to done zero callbacks list
	p_callbacks->m_doneZeroList.append(callback);
	// call it inmediatly if already resolved
	if (m_state == QDeferredState::RESOLVED)
	{
		callback();
	}
}

template<class ...Types>
void QDeferredData<Types...>::failZero(std::function<void()> callback)
{
	// add object for thread if does not exists
	auto p_callbacks = this->getCallbaksForThread();
	// append to fail zero callbacks list
	p_callbacks->m_failZeroList.append(callback);
	// call it inmediatly if already rejected
	if (m_state == QDeferredState::REJECTED)
	{
		callback();
	}
}

template<class ...Types>
void QDeferredData<Types...>::executeZero(QList< std::function<void()> > &listCallbacks)
{
	// loop all callbacks
	for (int i = 0; i < listCallbacks.length(); i++)
	{
		// call each callback without arguments
		listCallbacks.at(i)();
	}
}

// https://stackoverflow.com/questions/13559756/declaring-a-struct-in-a-template-class-undefined-for-member-functions
template<class ...Types>
typename QDeferredData<Types...>::DeferredAllCallbacks * QDeferredData<Types...>::getCallbaksForThread()
{
	QMutexLocker locker(&m_mutex);
	// get current thread
	QThread * p_currThd = QThread::currentThread();
	// if not in list then...
	if (!m_callbacksMap.contains(p_currThd))
	{
		QDeferredProxyObject * p_obj = QDeferredDataBase::getObjectForThread(p_currThd);
		// wait until object destroyed to remove callbacks struct
		// NOTE : need to disconnect these connections to avoid memory leaks due to lambda memory allocations
		m_connectionList.append(QObject::connect(p_obj, &QObject::destroyed, [&, p_currThd]() {
			// delete callbacks when thread gets deleted
			auto p_callbacksToDel = m_callbacksMap.take(p_currThd);
			// NOTE : m_connectionList.append not necessary here because conneciton will auto delete when p_currThd gets deleted
			QObject::connect(p_currThd, &QObject::destroyed, [p_callbacksToDel]() {
				delete p_callbacksToDel;
			});			
		}));
		// add callbacks struct to maps
		m_callbacksMap[p_currThd] = new QDeferredData<Types...>::DeferredAllCallbacks;
	};
	// return
	return m_callbacksMap[p_currThd];
}


#endif // QDEFERREDDATA_H