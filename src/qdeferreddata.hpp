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

#include <QDebug>

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

// [GCC_DEF_FIX]
namespace GCC_DEF_FIX {
	template<class ...Types>
	void finishedFunctionTemplate(std::function<void(Types(&...args))> funcFinish, Types(&...args))
	{
		funcFinish(args...);
	}
}

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
	void done(const std::function<void(Types (&...args))> &callback,  // by copy would be <Types... arg>
		      const Qt::ConnectionType &connection);
	// fail method
	void fail(const std::function<void(Types(&...args))> &callback, 
		      const Qt::ConnectionType &connection);
	// progress method
	void progress(const std::function<void(Types(&...args))> &callback, 
		          const Qt::ConnectionType &connection);

	// provider API

	// resolve method (ref added to avoid last reference deletion before callbacks execution)
	void resolve(QDeferred<Types...> ref, Types(&...args));
	// reject method
	void reject(QDeferred<Types...> ref, Types(&...args));
	// notify method
	void notify(QDeferred<Types...> ref, Types(&...args));

	// internal API

	// done method with zero arguments
	void doneZero(const std::function<void()> &callback, 
		          const Qt::ConnectionType &connection);
	// fail method with zero arguments
	void failZero(const std::function<void()> &callback, 
		          const Qt::ConnectionType &connection);

	// reject method with zero arguments (only to be used internally for the 'then' propagation mechanism)
	void rejectZero(QDeferred<Types...> ref);

	// when memory
	int m_whenCount = 0;

private:
	// struct to store callback data
	struct CallbackData
	{
		std::function<void(Types(&...args))> callback;
		Qt::ConnectionType                   connection;
	};
	// struct to store zero callback data
	struct CallbackDataZero
	{
		std::function<void()> callback;
		Qt::ConnectionType    connection;
	};
	// structure to contain callbacks (one instance per thread in m_callbacksMap)
	struct DeferredAllCallbacks 
	{
		QList< CallbackData     > m_doneList    ;
		QList< CallbackData     > m_failList    ;
		QList< CallbackData     > m_progressList;
		QList< CallbackDataZero > m_doneZeroList;
		QList< CallbackDataZero > m_failZeroList;
	};
	// members
	std::function<void(std::function<void(Types(&...args))>)> m_finishedFunction;
	QMap< QThread *, DeferredAllCallbacks * > m_callbacksMap;
	QDeferredState m_state;
	QMutex         m_mutex;
	QList<QMetaObject::Connection> m_connectionList;
	// methods
	DeferredAllCallbacks * getCallbaksForThread();
};

template<class ...Types>
QDeferredData<Types...>::QDeferredData() :
	m_mutex(QMutex::Recursive)
{
	m_state = QDeferredState::PENDING;
	m_finishedFunction = nullptr;
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
void QDeferredData<Types...>::done(const std::function<void(Types(&...args))> &callback, 
	                               const Qt::ConnectionType &connection/* = Qt::AutoConnection*/)
{
	// call it inmediatly if already resolved
	QMutexLocker locker(&m_mutex);
	if (m_state == QDeferredState::RESOLVED)
	{
		Q_ASSERT(m_finishedFunction);
		m_finishedFunction(callback);
	}
	else
	{
		// add object for thread if does not exists
		auto p_callbacks = this->getCallbaksForThread();
		// append to done callbacks list
		p_callbacks->m_doneList.append({ callback, connection });
	}
}

template<class ...Types>
void QDeferredData<Types...>::fail(const std::function<void(Types(&...args))> &callback, 
	                               const Qt::ConnectionType &connection/* = Qt::AutoConnection*/)
{
	// call it inmediatly if already rejected
	QMutexLocker locker(&m_mutex);
	// NOTE : m_finishedFunction can be nullptr here if m_state was set to QDeferredState::RESOLVED
	//        due to call to ::rejectZero before ::resolve or ::reject are called, in which case 
	//        this callback should not be called, since there are not arguments to call it.
	//        This condition happens, for example, when a new deferred object is returned by 'then'
	//        method of another deferred that has been already rejected and this new deferred object 
	//        subscribes a fail callback. Thats how we arrive here with a m_finishedFunction == nullptr
	if (m_finishedFunction && m_state == QDeferredState::REJECTED)
	{
		m_finishedFunction(callback);
	}
	else
	{
		// add object for thread if does not exists
		auto p_callbacks = this->getCallbaksForThread();
		// append to fail callbacks list
		p_callbacks->m_failList.append({ callback, connection });
	}
}

template<class ...Types>
void QDeferredData<Types...>::progress(const std::function<void(Types(&...args))> &callback,
	                                   const Qt::ConnectionType &connection/* = Qt::AutoConnection*/)
{
	// add object for thread if does not exists
	auto p_callbacks = this->getCallbaksForThread();
	// append to progress callbacks list
	p_callbacks->m_progressList.append({ callback, connection });
}

template<class ...Types>
void QDeferredData<Types...>::resolve(QDeferred<Types...> ref, Types(&...args))
{
	QMutexLocker locker(&m_mutex);
	// early exit if deferred has been already resolved or rejected
	Q_ASSERT_X(m_state == QDeferredState::PENDING, "QDeferred", "Cannot resolve already processed deferred object.");
	if (m_state != QDeferredState::PENDING)
	{
		qWarning() << "Cannot resolve already processed deferred object.";
		return;
	}
	// change state
	m_state = QDeferredState::RESOLVED;
	// set finished function (used to cache variadic args as a copy to be able to exec funcs added after resolve)
	m_finishedFunction = std::bind(GCC_DEF_FIX::finishedFunctionTemplate<Types...>, std::placeholders::_1, args...);

	// for each thread where there are callbacks to be called
	QMapIterator< QThread *, DeferredAllCallbacks *> i(m_callbacksMap);
	while (i.hasNext()) 
	{
		i.next();
		// get data for current thread
		auto p_currThread     = i.key();
		auto p_currObject     = QDeferredDataBase::getObjectForThread(p_currThread);
		auto p_currCallbacks  = m_callbacksMap[p_currThread];

		// execute all done callbacks
		for (int k = 0; k < p_currCallbacks->m_doneList.count(); k++)
		{
			auto &currConnection = p_currCallbacks->m_doneList[k].connection;
			auto &currCallback   = p_currCallbacks->m_doneList[k].callback;
			// execute according to connection type
			if (currConnection == Qt::DirectConnection || (currConnection == Qt::AutoConnection && p_currThread == QThread::currentThread()))
			{
				Q_ASSERT(m_finishedFunction);
				// call directly with arguments
				m_finishedFunction(currCallback);
			}
			else if (currConnection == Qt::QueuedConnection || (currConnection == Qt::AutoConnection && p_currThread != QThread::currentThread()))
			{
				// create object in heap and assign function (event loop takes ownership and deletes it later)
				QDeferredProxyEvent * p_Evt = new QDeferredProxyEvent;
				p_Evt->m_eventFunc = [ref, this, currCallback]() mutable {
					Q_ASSERT(m_finishedFunction);
					// call in thread with arguments
					m_finishedFunction(currCallback);
					// unused, but we need it to keep at least one reference until all callbacks are executed
					Q_UNUSED(ref)
				};
				// post event for object with correct thread affinity
				QCoreApplication::postEvent(p_currObject, p_Evt);
			}
			else
			{
				Q_ASSERT_X(false, "QDeferredData<Types...>::resolve", "Unsupported connection type.");
			}
		} // done callbacks

		// execute all done zero callbacks
		for (int k = 0; k < p_currCallbacks->m_doneZeroList.count(); k++)
		{
			auto &currConnection = p_currCallbacks->m_doneZeroList[k].connection;
			auto &currCallback   = p_currCallbacks->m_doneZeroList[k].callback;
			// execute according to connection type
			if (currConnection == Qt::DirectConnection || (currConnection == Qt::AutoConnection && p_currThread == QThread::currentThread()))
			{
				// call directly
				currCallback();
			}
			else if (currConnection == Qt::QueuedConnection || (currConnection == Qt::AutoConnection && p_currThread != QThread::currentThread()))
			{
				// create object in heap and assign function (event loop takes ownership and deletes it later)
				QDeferredProxyEvent * p_Evt = new QDeferredProxyEvent;
				p_Evt->m_eventFunc = [ref, this, currCallback]() mutable {
					// call in thread
					currCallback();
					// unused, but we need it to keep at least one reference until all callbacks are executed
					Q_UNUSED(ref)
				};
				// post event for object with correct thread affinity
				QCoreApplication::postEvent(p_currObject, p_Evt);
			}
			else
			{
				Q_ASSERT_X(false, "QDeferredData<Types...>::resolve", "Unsupported connection type.");
			}
		} // done zero callbacks

		// clear callbacks since wont be used again, except progress because it can be used continously
		p_currCallbacks->m_doneList.clear();
		p_currCallbacks->m_failList.clear();
		//p_currCallbacks->m_progressList.clear(); // NOTE : do not clear
		p_currCallbacks->m_doneZeroList.clear();
		p_currCallbacks->m_failZeroList.clear();

	} // for each thread
}

template<class ...Types>
void QDeferredData<Types...>::reject(QDeferred<Types...> ref, Types(&...args))
{
	QMutexLocker locker(&m_mutex);
	// early exit if deferred has been already resolved or rejected
	Q_ASSERT_X(m_state == QDeferredState::PENDING, "QDeferred", "Cannot reject already processed deferred object.");
	if (m_state != QDeferredState::PENDING)
	{
		qWarning() << "Cannot reject already processed deferred object.";
		return;
	}
	// change state
	m_state = QDeferredState::REJECTED;
	// set finished function (used to cache variadic args as a copy to be able to exec funcs added after reject)
	m_finishedFunction = std::bind(GCC_DEF_FIX::finishedFunctionTemplate<Types...>, std::placeholders::_1, args...);

	// for each thread where there are callbacks to be called
	QMapIterator< QThread *, DeferredAllCallbacks *> i(m_callbacksMap);
	while (i.hasNext()) 
	{
		i.next();
		// get data for current thread
		auto p_currThread     = i.key();
		auto p_currObject     = QDeferredDataBase::getObjectForThread(p_currThread);
		auto p_currCallbacks  = m_callbacksMap[p_currThread];

		// execute all fail callbacks
		for (int k = 0; k < p_currCallbacks->m_failList.count(); k++)
		{
			auto &currConnection = p_currCallbacks->m_failList[k].connection;
			auto &currCallback   = p_currCallbacks->m_failList[k].callback;
			// execute according to connection type
			if (currConnection == Qt::DirectConnection || (currConnection == Qt::AutoConnection && p_currThread == QThread::currentThread()))
			{
				Q_ASSERT(m_finishedFunction);
				// call directly with arguments
				m_finishedFunction(currCallback);
			}
			else if (currConnection == Qt::QueuedConnection || (currConnection == Qt::AutoConnection && p_currThread != QThread::currentThread()))
			{
				// create object in heap and assign function (event loop takes ownership and deletes it later)
				QDeferredProxyEvent * p_Evt = new QDeferredProxyEvent;
				p_Evt->m_eventFunc = [ref, this, currCallback]() mutable {
					Q_ASSERT(m_finishedFunction);
					// call in thread with arguments
					m_finishedFunction(currCallback);
					// unused, but we need it to keep at least one reference until all callbacks are executed
					Q_UNUSED(ref)
				};
				// post event for object with correct thread affinity
				QCoreApplication::postEvent(p_currObject, p_Evt);
			}
			else
			{
				Q_ASSERT_X(false, "QDeferredData<Types...>::resolve", "Unsupported connection type.");
			}
		} // fail callbacks

		// execute all fail zero callbacks
		for (int k = 0; k < p_currCallbacks->m_failZeroList.count(); k++)
		{
			auto &currConnection = p_currCallbacks->m_failZeroList[k].connection;
			auto &currCallback   = p_currCallbacks->m_failZeroList[k].callback;
			// execute according to connection type
			if (currConnection == Qt::DirectConnection || (currConnection == Qt::AutoConnection && p_currThread == QThread::currentThread()))
			{
				// call directly
				currCallback();
			}
			else if (currConnection == Qt::QueuedConnection || (currConnection == Qt::AutoConnection && p_currThread != QThread::currentThread()))
			{
				// create object in heap and assign function (event loop takes ownership and deletes it later)
				QDeferredProxyEvent * p_Evt = new QDeferredProxyEvent;
				p_Evt->m_eventFunc = [ref, this, currCallback]() mutable {
					// call in thread
					currCallback();
					// unused, but we need it to keep at least one reference until all callbacks are executed
					Q_UNUSED(ref)
				};
				// post event for object with correct thread affinity
				QCoreApplication::postEvent(p_currObject, p_Evt);
			}
			else
			{
				Q_ASSERT_X(false, "QDeferredData<Types...>::reject", "Unsupported connection type.");
			}
		} // fail zero callbacks

		// clear callbacks since wont be used again, except progress because it can be used continously
		p_currCallbacks->m_doneList.clear();
		p_currCallbacks->m_failList.clear();
		//p_currCallbacks->m_progressList.clear(); // NOTE : do not clear
		p_currCallbacks->m_doneZeroList.clear();
		p_currCallbacks->m_failZeroList.clear();

	} // for each thread
}

template<class ...Types>
void QDeferredData<Types...>::rejectZero(QDeferred<Types...> ref)
{
	QMutexLocker locker(&m_mutex);
	// early exit if deferred has been already resolved or rejected
	Q_ASSERT_X(m_state == QDeferredState::PENDING, "QDeferred", "Cannot reject already processed deferred object.");
	if (m_state != QDeferredState::PENDING)
	{
		qWarning() << "Cannot reject already processed deferred object.";
		return;
	}
	// change state
	m_state = QDeferredState::REJECTED;

	// for each thread where there are callbacks to be called
	QMapIterator< QThread *, DeferredAllCallbacks *> i(m_callbacksMap);
	while (i.hasNext())
	{
		i.next();
		// set test function
		auto p_currThread    = i.key();
		auto p_currObject    = QDeferredDataBase::getObjectForThread(p_currThread);
		auto p_currCallbacks = m_callbacksMap[p_currThread];

		// execute all fail zero callbacks
		for (int k = 0; k < p_currCallbacks->m_failZeroList.count(); k++)
		{
			auto &currConnection = p_currCallbacks->m_failZeroList[k].connection;
			auto &currCallback   = p_currCallbacks->m_failZeroList[k].callback;
			// execute according to connection type
			if (currConnection == Qt::DirectConnection || (currConnection == Qt::AutoConnection && p_currThread == QThread::currentThread()))
			{
				// call directly
				currCallback();
			}
			else if (currConnection == Qt::QueuedConnection || (currConnection == Qt::AutoConnection && p_currThread != QThread::currentThread()))
			{
				// create object in heap and assign function (event loop takes ownership and deletes it later)
				QDeferredProxyEvent * p_Evt = new QDeferredProxyEvent;
				p_Evt->m_eventFunc = [ref, this, currCallback]() mutable {
					// call in thread
					currCallback();
					// unused, but we need it to keep at least one reference until all callbacks are executed
					Q_UNUSED(ref)
				};
				// post event for object with correct thread affinity
				QCoreApplication::postEvent(p_currObject, p_Evt);
			}
			else
			{
				Q_ASSERT_X(false, "QDeferredData<Types...>::rejectZero", "Unsupported connection type.");
			}
		} // fail zero callbacks

		// clear only 'zero' callbacks since wont be used again
		p_currCallbacks->m_doneZeroList.clear();
		p_currCallbacks->m_failZeroList.clear();
	}
}


template<class ...Types>
void QDeferredData<Types...>::notify(QDeferred<Types...> ref, Types(&...args))
{
	QMutexLocker locker(&m_mutex);
	// early exit if deferred has been already resolved or rejected
	Q_ASSERT_X(m_state == QDeferredState::PENDING, "QDeferred", "Cannot notify already processed deferred object.");
	if (m_state != QDeferredState::PENDING)
	{
		qWarning() << "Cannot notify already processed deferred object.";
		return;
	}

	// with notify we cannot use execute -> m_finishedFunction combo because; if notify-events are
	// not processed inmediatly after, then progress callbacks will be called with incorrect arguments
	// (with the last agruments that were given to the last notify call, e.g. "3, 3, 3", instead of "1, 2, 3")
	auto funcCacheArgs = std::bind(GCC_DEF_FIX::finishedFunctionTemplate<Types...>, std::placeholders::_1, args...);

	// for each thread where there are callbacks to be called
	QMapIterator< QThread *, DeferredAllCallbacks *> i(m_callbacksMap);
	while (i.hasNext())
	{
		i.next();
		// set test function
		auto p_currThread    = i.key();
		auto p_currObject    = QDeferredDataBase::getObjectForThread(p_currThread);
		auto p_currCallbacks = m_callbacksMap[p_currThread];

		for (int k = 0; k < p_currCallbacks->m_progressList.count(); k++)
		{
			auto &currConnection = p_currCallbacks->m_progressList[k].connection;
			auto &currCallback   = p_currCallbacks->m_progressList[k].callback;
			// execute according to connection type
			if (currConnection == Qt::DirectConnection || (currConnection == Qt::AutoConnection && p_currThread == QThread::currentThread()))
			{
				// call directly
				funcCacheArgs(currCallback);
			}
			else if (currConnection == Qt::QueuedConnection || (currConnection == Qt::AutoConnection && p_currThread != QThread::currentThread()))
			{
				// create object in heap and assign function (event loop takes ownership and deletes it later)
				QDeferredProxyEvent * p_Evt = new QDeferredProxyEvent;
				p_Evt->m_eventFunc = [ref, this, funcCacheArgs, currCallback]() mutable {
					// call in thread
					funcCacheArgs(currCallback);
					// unused, but we need it to keep at least one reference until all callbacks are executed
					Q_UNUSED(ref)
				};
				// post event for object with correct thread affinity
				QCoreApplication::postEvent(p_currObject, p_Evt);
			}
			else
			{
				Q_ASSERT_X(false, "QDeferredData<Types...>::notify", "Unsupported connection type.");
			}
		} // progress callbacks
	} // for each thread
}

template<class ...Types>
void QDeferredData<Types...>::doneZero(const std::function<void()> &callback,
	                                   const Qt::ConnectionType &connection/* = Qt::AutoConnection*/)
{
	// call it inmediatly if already resolved
	QMutexLocker locker(&m_mutex);
	if (m_state == QDeferredState::RESOLVED)
	{
		callback();
	}
	else
	{
		// add object for thread if does not exists
		auto p_callbacks = this->getCallbaksForThread();
		// append to done zero callbacks list
		p_callbacks->m_doneZeroList.append({ callback, connection });
	}
}

template<class ...Types>
void QDeferredData<Types...>::failZero(const std::function<void()> &callback,
	                                   const Qt::ConnectionType &connection/* = Qt::AutoConnection*/)
{
	// call it inmediatly if already rejected
	QMutexLocker locker(&m_mutex);
	if (m_state == QDeferredState::REJECTED)
	{
		callback();
	}
	else
	{
		// add object for thread if does not exists
		auto p_callbacks = this->getCallbaksForThread();
		// append to fail zero callbacks list
		p_callbacks->m_failZeroList.append({ callback, connection });
	}
}

// https://stackoverflow.com/questions/13559756/declaring-a-struct-in-a-template-class-undefined-for-member-functions
template<class ...Types>
typename QDeferredData<Types...>::DeferredAllCallbacks * QDeferredData<Types...>::getCallbaksForThread()
{
	QMutexLocker locker(&m_mutex);
	// get current thread
	QThread * p_currThd = QThread::currentThread();
	// if not in list...
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
