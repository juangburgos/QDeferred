#ifndef QDEFERREDDATA_H
#define QDEFERREDDATA_H

#include <QCoreApplication>
#include <QSharedData>
#include <QList>
#include <QThread>
#include <functional>

#include "qdeferredproxyobject.h"

enum QDeferredState
{
	PENDING,
	RESOLVED,
	REJECTED
};

template<class ...Types>
class QDeferredData : public QSharedData
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

	// provider API

	// resolve method
	void resolve(Types(&...args));
	// reject method
	void reject(Types(&...args));

	// internal API

	// done method with zero arguments
	void doneZero(std::function<void()> callback);
	// fail method with zero arguments
	void failZero(std::function<void()> callback);

	// when memory
	int m_whenCount = 0;

private:
	struct DeferredAllCallbacks {
		QList< std::function<void(Types(&...args))> > m_doneList;
		QList< std::function<void(Types(&...args))> > m_failList;
		QList< std::function<void(Types(&...args))> > m_thenList;
		QList< std::function<void()               > > m_doneZeroList;
		QList< std::function<void()               > > m_failZeroList;
	};
	// members
	QMap< QThread *, QDeferredProxyObject * > m_threadMap;
	QMap< QThread *, DeferredAllCallbacks * > m_callbacksMap;
	QDeferredState m_state;
	std::function<void(std::function<void(Types(&...args))>)> m_finishedFunction;
	// methods
	void execute    (QList< std::function<void(Types(&...args))> > &listCallbacks, Types(&...args));
	void executeZero(QList< std::function<void()               > > &listCallbacks);
	DeferredAllCallbacks * getCallbaksForThread();
};

template<class ...Types>
QDeferredData<Types...>::QDeferredData()
{
	m_state = QDeferredState::PENDING;
}

template<class ...Types>
QDeferredData<Types...>::~QDeferredData()
{
	// delete all memory allocated on heap
	qDeleteAll(m_threadMap);
	qDeleteAll(m_callbacksMap);
}

template<class ...Types>
QDeferredData<Types...>::QDeferredData(const QDeferredData &other) : QSharedData(other),
m_whenCount(other.m_whenCount),
m_threadMap(other.m_threadMap),
m_callbacksMap(other.m_callbacksMap),
m_state(other.m_state),
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
	if (m_state == QDeferredState::RESOLVED || m_state == QDeferredState::REJECTED)
	{
		m_finishedFunction(callback);
	}
}

template<class ...Types>
void QDeferredData<Types...>::resolve(Types(&...args))
{
	// early exit if deferred has been already resolved or rejected
	if (m_state != QDeferredState::PENDING)
	{
		qWarning() << "Cannot resolve already processed deferred object.";
	}
	// change state
	m_state = QDeferredState::RESOLVED;
	// set finished function (used to cache variadic args as a copy to be able to exec funcs added after resolve)
	m_finishedFunction = [args...](std::function<void(Types(&...args))> callback) mutable {
		callback(args...);
	};
	// for each thread where there are callbacks to be called
	QMapIterator<QThread *, QDeferredProxyObject *> i(m_threadMap);
	while (i.hasNext()) 
	{
		i.next();
		// set test function
		auto p_currThread     = i.key();
		auto p_currObject     = m_threadMap[p_currThread];
		auto p_currCallbacks  = m_callbacksMap[p_currThread];
		p_currObject->m_perThreadFunc = [/* p_currThread, */this, p_currCallbacks, args...]() mutable {
			//qDebug() << "[INFO] Callback for thread " << p_currThread << " exec in thread " << QThread::currentThread();
			// execute all done callbacks
			this->execute(p_currCallbacks->m_doneList, args...); // DONE
			// execute all then callbacks
			this->execute(p_currCallbacks->m_thenList, args...); // THEN
			// loop all done zero callbacks
			this->executeZero(p_currCallbacks->m_doneZeroList);  // DONE
		};
		// post event for object (create in heap, event loop takes ownership and delets it later)
		QDeferredProxyEvent * p_Evt = new QDeferredProxyEvent;
		QCoreApplication::postEvent(p_currObject, p_Evt);
	}	
}

template<class ...Types>
void QDeferredData<Types...>::reject(Types(&...args))
{
	// early exit if deferred has been already resolved or rejected
	if (m_state != QDeferredState::PENDING)
	{
		qWarning() << "Cannot reject already processed deferred object.";
	}
	// change state
	m_state = QDeferredState::REJECTED;
	// set finished function (used to cache variadic args as a copy to be able to exec funcs added after reject)
	m_finishedFunction = [args...](std::function<void(Types(&...args))> callback) mutable {
		callback(args...);
	};
	// for each thread where there are callbacks to be called
	QMapIterator<QThread *, QDeferredProxyObject *> i(m_threadMap);
	while (i.hasNext())
	{
		i.next();
		// set test function
		auto p_currThread = i.key();
		auto p_currObject = m_threadMap[p_currThread];
		auto p_currCallbacks = m_callbacksMap[p_currThread];
		p_currObject->m_perThreadFunc = [/* p_currThread, */this, p_currCallbacks, args...]() mutable {
			//qDebug() << "[INFO] Callback for thread " << p_currThread << " exec in thread " << QThread::currentThread();
			// execute all done callbacks
			this->execute(p_currCallbacks->m_failList, args...); // FAIL
			// execute all then callbacks
			this->execute(p_currCallbacks->m_thenList, args...); // THEN
			// loop all done zero callbacks
			this->executeZero(p_currCallbacks->m_failZeroList);  // FAIL
		};
		// post event for object (create in heap, event loop takes ownership and delets it later)
		QDeferredProxyEvent * p_Evt = new QDeferredProxyEvent;
		QCoreApplication::postEvent(p_currObject, p_Evt);
	}
}

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
	// get current thread
	QThread * p_currThd = QThread::currentThread();
	// if not in list then...
	if (!m_threadMap.contains(p_currThd))
	{
		// subscribe to finish
		QObject::connect(p_currThd, &QThread::finished, [&]() {
			// if finished
			// remove from all maps
			auto p_objToDelete = m_threadMap.take(p_currThd);
			// wait until object destroyed to remove callbacks struct
			QObject::connect(p_objToDelete, &QObject::destroyed, [&]() {
				delete m_callbacksMap.take(p_currThd);
			});
			// mark the object ofr deletion
			p_objToDelete->deleteLater();
		});
		// add to all maps
		m_threadMap[p_currThd] = new QDeferredProxyObject;
		m_callbacksMap[p_currThd] = new QDeferredData<Types...>::DeferredAllCallbacks;
	}
	// return
	return m_callbacksMap[p_currThd];
}

#endif // QDEFERREDDATA_H