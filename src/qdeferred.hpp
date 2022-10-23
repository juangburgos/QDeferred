#ifndef QDEFERRED_H
#define QDEFERRED_H

#include <QExplicitlySharedDataPointer>
#include <QList>
#include <QTimer>
#include <functional>
#include <QDebug>

#include "qdeferreddata.hpp"

template<class ...Types>
class QDeferred
{

public:
	// constructors
	QDeferred();
	QDeferred(const QDeferred<Types...> &other);
	QDeferred &operator=(const QDeferred<Types...> &rhs);
	~QDeferred();

	// wrapper consumer API (with chaning)

	// get state method
	QDeferredState state() const;

	// done method
	QDeferred<Types...> done(
		const std::function<void(Types(...args))> &callback,
		const Qt::ConnectionType                  &connection = Qt::AutoConnection);

	// fail method
	QDeferred<Types...> fail(
		const std::function<void(Types(...args))> &callback,
		const Qt::ConnectionType                  &connection = Qt::AutoConnection);

	// then method
	template<class ...RetTypes, typename T>
	QDeferred<RetTypes...> then(
		const T                  &doneCallback,
		const Qt::ConnectionType &connection = Qt::AutoConnection);
	// then method specialization
	template<class ...RetTypes>
	QDeferred<RetTypes...> then(
		const std::function<QDeferred<RetTypes...>(Types(...args))> &doneCallback,
		const Qt::ConnectionType                                    &connection = Qt::AutoConnection);

	// then method
	template<class ...RetTypes, typename T>
	QDeferred<RetTypes...> then(
		const T                      &doneCallback,
		const std::function<void()>  &failCallback,
		const Qt::ConnectionType     &connection = Qt::AutoConnection);
	// then method specialization
	template<class ...RetTypes>
	QDeferred<RetTypes...> then(
		const std::function<QDeferred<RetTypes...>(Types(...args))> &doneCallback,
		const std::function<void()>                                 &failCallback,
		const Qt::ConnectionType                                    &connection = Qt::AutoConnection);

	// progress method
	QDeferred<Types...> progress(
		const std::function<void(Types(...args))> &callback,
		const Qt::ConnectionType                  &connection = Qt::AutoConnection);

	// extra consume API (static)

	// NOTE : there is no case in setting a Qt::ConnectionType in when method because
	//        we never know which deferred in which thread will be the last one to be resolved/rejected
	template <class ...OtherTypes, typename... Rest>
	static QDeferred<> when(QDeferred<OtherTypes...> t, Rest... rest);

	// can pass a container (implementing begin/end iterator and ::size()) 
	// instead of passing variadic arguments one by one
	template<template<class> class Container, class ...OtherTypes>
	static QDeferred<> when(const Container<QDeferred<OtherTypes...>>& deferList);

	// block current thread until deferred object gets resolved/rejected
	// NOTE : since current thread is blocked, the deferred object must be
	//        resolved/rejected in a different thread
	template <class ...OtherTypes, typename... Rest>
	static bool await(QDeferred<OtherTypes...> t, Rest... rest);

	// syntactic sugar for container of deferred objects
	template<template<class> class Container, class ...OtherTypes>
	static bool await(const Container<QDeferred<OtherTypes...>>& deferList);

	// wrapper provider API

	// resolve method
	void resolve(Types(...args));
	// reject method
	void reject(Types(...args));
	// notify method
	void notify(Types(...args));

	// reject method with zero arguments (only to be used internally for the 'then' propagation mechanism)
	void rejectZero();

protected:
	QExplicitlySharedDataPointer<QDeferredData<Types...>> m_data;

private:
	// friend classes

	friend class QDeferredDataBase;

	// internal methods

	// get when count method
	int  getWhenCount();
	// set when count method
	void setWhenCount(int whenCount);

	// done method with zero arguments
	void doneZero(const std::function<void()> &callback,
		const Qt::ConnectionType &connection = Qt::AutoConnection);
	// fail method with zero arguments
	void failZero(const std::function<void()> &callback,
		const Qt::ConnectionType &connection = Qt::AutoConnection);

	// then method specialization internal
	template<class ...RetTypes>
	QDeferred<RetTypes...> thenAlias(
		const std::function<QDeferred<RetTypes...>(Types(...args))> &doneCallback,
		const Qt::ConnectionType                                    &connection = Qt::AutoConnection);

	// then method specialization internal
	template<class ...RetTypes>
	QDeferred<RetTypes...> thenAlias(
		const std::function<QDeferred<RetTypes...>(Types(...args))> &doneCallback,
		const std::function<void()>                                 &failCallback,
		const Qt::ConnectionType                                    &connection = Qt::AutoConnection);

	// internal await requires simple QDefer
	static bool awaitInternal(const QDeferred<>& defer);
};

// alias for no argument types
using QDefer = QDeferred<>;

template<class ...Types>
QDeferred<Types...>::QDeferred() : m_data(nullptr)
{
	m_data = QExplicitlySharedDataPointer<QDeferredData<Types...>>(new QDeferredData<Types...>());
}

template<class ...Types>
QDeferred<Types...>::QDeferred(const QDeferred<Types...> &other) : m_data(other.m_data)
{
	m_data.reset();
	m_data = other.m_data;
}

template<class ...Types>
QDeferred<Types...> &QDeferred<Types...>::operator=(const QDeferred<Types...> &rhs)
{
	if (this != &rhs) {
		m_data.reset();
		m_data.operator=(rhs.m_data);
	}
	return *this;
}

template<class ...Types>
QDeferred<Types...>::~QDeferred()
{
	m_data.reset();
}

template<class ...Types>
QDeferredState QDeferred<Types...>::state() const
{
	return m_data->state();
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::done(
	const std::function<void(Types(...args))> & callback, 
	const Qt::ConnectionType                  & connection/* = Qt::AutoConnection*/)
{
	// check if valid
	Q_ASSERT_X(callback, "Deferred done method.", "Invalid done callback argument");
	m_data->done(callback, connection);
	return *this;
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::fail(const std::function<void(Types(...args))> &callback,
	const Qt::ConnectionType &connection/* = Qt::AutoConnection*/)
{
	// check if valid
	Q_ASSERT_X(callback, "Deferred fail method.", "Invalid fail callback argument");
	m_data->fail(callback, connection);
	return *this;
}

template<class ...Types>
template<class ...RetTypes, typename T>
QDeferred<RetTypes...> QDeferred<Types...>::then(
	const T                  &doneCallback,
	const Qt::ConnectionType &connection/* = Qt::AutoConnection*/)
{
	return this->thenAlias<RetTypes...>((std::function<QDeferred<RetTypes...>(Types...)>)doneCallback, connection);
}

template<class ...Types>
template<class ...RetTypes>
QDeferred<RetTypes...> QDeferred<Types...>::then(
	const std::function<QDeferred<RetTypes...>(Types(...args))> & doneCallback,
	const Qt::ConnectionType                                    & connection/* = Qt::AutoConnection*/)
{
	return this->thenAlias<RetTypes...>(doneCallback, connection);
}

template<class ...Types>
template<class ...RetTypes>
QDeferred<RetTypes...> QDeferred<Types...>::thenAlias(
	const std::function<QDeferred<RetTypes...>(Types(...args))> & doneCallback,
	const Qt::ConnectionType                                    & connection/* = Qt::AutoConnection*/)
{
	// check if valid
	Q_ASSERT_X(doneCallback, "Deferred then method.", "Invalid done callback as first argument");

	// create deferred to return
	QDeferred<RetTypes...> retPromise;

	std::function<void(Types...)> intermediateDoneCallback = 
	[doneCallback, retPromise](Types... args1) mutable {
		// when done execute user callback, then when user deferred and when done...
		doneCallback(args1...)
		.done([retPromise](RetTypes... args2) mutable {
			// resolve returned deferred
			retPromise.resolve(args2...);
		})
		.fail([retPromise](RetTypes... args2) mutable {
			// reject returned deferred
			retPromise.reject(args2...);
		})
		.progress([retPromise](RetTypes... args2) mutable {
			// notify returned deferred
			retPromise.notify(args2...);
		});
	};

	// add intermediate done nameless callback
	m_data->done(intermediateDoneCallback, connection);

	// allow propagation
	m_data->failZero([retPromise]() mutable {
		// reject zero
		retPromise.rejectZero();
	}, connection);

	// return new deferred
	return retPromise;
}

template<class ...Types>
template<class ...RetTypes, typename T>
QDeferred<RetTypes...> QDeferred<Types...>::then(
	const T                     &doneCallback,
	const std::function<void()> &failCallback,
	const Qt::ConnectionType    &connection/* = Qt::AutoConnection*/)
{
	return this->thenAlias<RetTypes...>((std::function<QDeferred<RetTypes...>(Types...)>)doneCallback, failCallback, connection);
}

template<class ...Types>
template<class ...RetTypes>
QDeferred<RetTypes...> QDeferred<Types...>::then(
	const std::function<QDeferred<RetTypes...>(Types(...args))> & doneCallback,
	const std::function<void()>                                 & failCallback, 
	const Qt::ConnectionType                                    & connection/* = Qt::AutoConnection*/)
{
	return this->thenAlias<RetTypes...>(doneCallback, failCallback, connection);
}

template<class ...Types>
template<class ...RetTypes>
QDeferred<RetTypes...> QDeferred<Types...>::thenAlias(
	const std::function<QDeferred<RetTypes...>(Types(...args))> & doneCallback,
	const std::function<void()>                                 & failCallback,
	const Qt::ConnectionType                                    & connection/* = Qt::AutoConnection*/)
{
	// check if valid
	Q_ASSERT_X(doneCallback, "Deferred then method.", "Invalid done callback as first argument");
	Q_ASSERT_X(failCallback, "Deferred then method.", "Invalid fail callback as second argument");

	// add fail zero (internal) callback
	m_data->failZero([failCallback]() mutable {
		// when fail zero execute user callback
		failCallback();
	}, connection);

	// call other
	return this->then<RetTypes...>(doneCallback, connection);
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::progress(const std::function<void(Types(...args))> &callback,
	const Qt::ConnectionType &connection/* = Qt::AutoConnection*/)
{
	// check if valid
	Q_ASSERT_X(callback, "Deferred progress method.", "Invalid progress callback argument");
	m_data->progress(callback, connection);
	return *this;
}

template<class ...Types>
void QDeferred<Types...>::resolve(Types(...args))
{
	// pass reference to this to at least have 1 reference until callbacks get executed
	m_data->resolve(*this, args...);
}

template<class ...Types>
void QDeferred<Types...>::reject(Types(...args))
{
	// pass reference to this to at least have 1 reference until callbacks get executed
	m_data->reject(*this, args...);
}

template<class ...Types>
void QDeferred<Types...>::rejectZero()
{
	// pass reference to this to at least have 1 reference until callbacks get executed
	m_data->rejectZero(*this);
}

template<class ...Types>
void QDeferred<Types...>::notify(Types(...args))
{
	// pass reference to this to at least have 1 reference until callbacks get executed
	m_data->notify(*this, args...);
}

template<class ...Types>
void QDeferred<Types...>::doneZero(const std::function<void()> &callback,
	const Qt::ConnectionType &connection/* = Qt::AutoConnection*/)
{
	// check if valid
	Q_ASSERT_X(callback, "Deferred doneZero method.", "Invalid doneZero callback argument");
	m_data->doneZero(callback, connection);
}

template<class ...Types>
void QDeferred<Types...>::failZero(const std::function<void()> &callback,
	const Qt::ConnectionType &connection/* = Qt::AutoConnection*/)
{
	// check if valid
	Q_ASSERT_X(callback, "Deferred failZero method.", "Invalid failZero callback argument");
	m_data->failZero(callback, connection);
}

/*
https://api.jquery.com/jQuery.when/
In the case where multiple Deferred objects are passed to jQuery.when(), the method returns the Promise from a
new "master" Deferred object that tracks the aggregate state of all the Deferreds it has been passed.
The method will resolve its master Deferred as soon as all the Deferreds resolve, or reject the master
Deferred as soon as one of the Deferreds is rejected.
*/
template<class ...Types>
template<class ...OtherTypes, class... Rest>
QDefer QDeferred<Types...>::when(QDeferred<OtherTypes...> t, Rest... rest)
{
	QDefer retDeferred;
	// setup necessary variables for expansion
	int    countArgs = sizeof...(Rest) + 1;
	// done callback, resolve if ALL done
	auto doneCallback = [retDeferred, countArgs]() mutable {
		// whenCount++
		retDeferred.setWhenCount(retDeferred.getWhenCount() + 1);
		int whenCount = retDeferred.getWhenCount();
		if (whenCount == countArgs)
		{
			retDeferred.resolve();
		}
	};
	// fail callback, reject if ONE fails
	auto failCallback = [retDeferred]() mutable {
		// can only reject once
		if (retDeferred.state() != QDeferredState::PENDING)
		{
			return;
		}
		retDeferred.reject();
	};
	// expand
	QDeferredDataBase::whenInternal(doneCallback, failCallback, t, rest...);
	// return deferred
	return retDeferred;
}

template<class ...Types>
template<template<class> class Container, class ...OtherTypes>
QDefer QDeferred<Types...>::when(const Container<QDeferred<OtherTypes...>>& deferList)
{
	QDefer retDeferred;
	// setup necessary variables for expansion
	int count = deferList.size();
	// done callback, resolve if ALL done
	auto doneCallback = [retDeferred, count]() mutable {
		// whenCount++
		retDeferred.setWhenCount(retDeferred.getWhenCount() + 1);
		int whenCount = retDeferred.getWhenCount();
		if (whenCount == count)
		{
			retDeferred.resolve();
		}
	};
	// fail callback, reject if ONE fails
	auto failCallback = [retDeferred]() mutable {
		// can only reject once
		if (retDeferred.state() != QDeferredState::PENDING)
		{
			return;
		}
		retDeferred.reject();
	};
	// call in friend class to access private methods
	QDeferredDataBase::whenInternal(doneCallback, failCallback, deferList);	
	// return deferred
	return retDeferred;
}

template<class ...Types>
template<class ...OtherTypes, typename ...Rest>
bool QDeferred<Types...>::await(QDeferred<OtherTypes...> t, Rest ...rest)
{
	return QDefer::awaitInternal(QDefer::when(t, rest...));
}

template<class ...Types>
template<template<class> class Container, class ...OtherTypes>
 bool QDeferred<Types...>::await(const Container<QDeferred<OtherTypes...>>& deferList)
{
	return QDefer::awaitInternal(QDefer::when(deferList));
}

template<class ...Types>
bool QDeferred<Types...>::awaitInternal(const QDeferred<>& defer)
{
	QEventLoop blockingEventLoop;
	// pass loop reference so it can be un-blocked in resolving/rejecting thread
	defer.m_data->m_blockingEventLoop = &blockingEventLoop;
	// handle case where thread gets stopped or deleted
	QThread* currThread = QThread::currentThread();
	QObject::connect(currThread, &QThread::finished, &blockingEventLoop, 
	[&blockingEventLoop]() {
		if (blockingEventLoop.isRunning())
		{
			blockingEventLoop.quit();
		}
	});
	QObject::connect(currThread, &QThread::destroyed, &blockingEventLoop, 
	[&blockingEventLoop]() {
		if (blockingEventLoop.isRunning())
		{
			blockingEventLoop.quit();
		}
	});
	// sometimes it is already resolved by the time we reach here
	if (defer.state() != QDeferredState::PENDING)
	{
		QTimer::singleShot(0, &blockingEventLoop, [&blockingEventLoop]() {
			if (blockingEventLoop.isRunning())
			{
				blockingEventLoop.quit();
			}
		});
	}
	// block until resolved/rejected
	blockingEventLoop.exec();
	// return whether resolved or rejected
	return defer.state() == QDeferredState::RESOLVED;
}

template<class ...Types>
void QDeferred<Types...>::setWhenCount(int whenCount)
{
	m_data->m_whenCount = whenCount;
}

template<class ...Types>
int QDeferred<Types...>::getWhenCount()
{
	return m_data->m_whenCount;
}

#endif // QDEFERRED_H
