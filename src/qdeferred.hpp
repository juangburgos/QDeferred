#ifndef QDEFERRED_H
#define QDEFERRED_H

#include <QExplicitlySharedDataPointer>
#include <QList>
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
	QDeferred<Types...> done(std::function<void(Types(...args))> callback); // by copy would be <Types... arg>

	// fail method
	QDeferred<Types...> fail(std::function<void(Types(...args))> callback);

	// then method
	template<class ...RetTypes, typename T>
	inline QDeferred<RetTypes...> then(T doneCallback) {
		return this->thenAlias<RetTypes...>(doneCallback);
	};
	template<class ...RetTypes>
	QDeferred<RetTypes...> thenAlias(std::function<QDeferred<RetTypes...>(Types(...args))> doneCallback);

	// then method
	template<class ...RetTypes, typename T>
	inline QDeferred<RetTypes...> then(T doneCallback, std::function<void()> failCallback) {
		return this->thenAlias<RetTypes...>(doneCallback, failCallback);
	};
	template<class ...RetTypes>
	QDeferred<RetTypes...> thenAlias(
		std::function<QDeferred<RetTypes...>(Types(...args))> doneCallback,
		std::function<void()>                                  failCallback);

	// progress method
	QDeferred<Types...> progress(std::function<void(Types(...args))> callback);

	// extra consume API (static)

	template <class ...OtherTypes, typename... Rest>
	static QDeferred<> when(QDeferred<OtherTypes...> t, Rest... rest);

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
	void doneZero(std::function<void()> callback);
	// fail method with zero arguments
	void failZero(std::function<void()> callback);

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
QDeferred<Types...> QDeferred<Types...>::done(std::function<void(Types(...args))> callback)
{
	// check if valid
	Q_ASSERT_X(callback, "Deferred done method.", "Invalid done callback argument");
	m_data->done(callback);
	return *this;
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::fail(std::function<void(Types(...args))> callback)
{
	// check if valid
	Q_ASSERT_X(callback, "Deferred fail method.", "Invalid fail callback argument");
	m_data->fail(callback);
	return *this;
}

template<class ...Types>
template<class ...RetTypes>
QDeferred<RetTypes...> QDeferred<Types...>::thenAlias(
	std::function<QDeferred<RetTypes...>(Types(...args))> doneCallback)
{
	// check if valid
	Q_ASSERT_X(doneCallback, "Deferred then method.", "Invalid done callback as first argument");

	// create deferred to return
	QDeferred<RetTypes...> retPromise;

	// add intermediate done nameless callback
	m_data->done([doneCallback, retPromise](Types(...args1)) mutable {
		// when done execute user callback, then when user deferred and when done...
		doneCallback(args1...)
			.done([retPromise](RetTypes(...args2)) mutable {
				// resolve returned deferred
				retPromise.resolve(args2...);
			})
			.fail([retPromise](RetTypes(...args2)) mutable {
				// reject returned deferred
				retPromise.reject(args2...);
			})
			.progress([retPromise](RetTypes(...args2)) mutable {
				// notify returned deferred
				retPromise.notify(args2...);
			});
	});

	// allow propagation
	m_data->failZero([retPromise]() mutable {
		// reject zero
		retPromise.rejectZero();
	});

	// return new deferred
	return retPromise;
}

template<class ...Types>
template<class ...RetTypes>
QDeferred<RetTypes...> QDeferred<Types...>::thenAlias(
	std::function<QDeferred<RetTypes...>(Types(...args))> doneCallback,
	std::function<void()>                                 failCallback)
{
	// check if valid
	Q_ASSERT_X(doneCallback, "Deferred then method.", "Invalid done callback as first argument");
	Q_ASSERT_X(failCallback, "Deferred then method.", "Invalid fail callback as second argument");

	// add fail zero (internal) callback
	m_data->failZero([failCallback]() mutable {
		// when fail zero execute user callback
		failCallback();
	});

	// call other
	return this->thenAlias<RetTypes...>(doneCallback);
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::progress(std::function<void(Types(...args))> callback)
{
	// check if valid
	Q_ASSERT_X(callback, "Deferred progress method.", "Invalid progress callback argument");
	m_data->progress(callback);
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
void QDeferred<Types...>::doneZero(std::function<void()> callback)
{
	// check if valid
	Q_ASSERT_X(callback, "Deferred doneZero method.", "Invalid doneZero callback argument");
	m_data->doneZero(callback);
}

template<class ...Types>
void QDeferred<Types...>::failZero(std::function<void()> callback)
{
	// check if valid
	Q_ASSERT_X(callback, "Deferred failZero method.", "Invalid failZero callback argument");
	m_data->failZero(callback);
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
	// setup necessary variables for expansion
	QDefer retDeferred;
	int    countArgs = sizeof...(Rest)+1;
	// done callback, resolve if ALL done
	auto doneCallback = [retDeferred, countArgs]() mutable {
		retDeferred.setWhenCount(retDeferred.getWhenCount() + 1); // whenCount++
		int whenCount = retDeferred.getWhenCount();
		if (whenCount == countArgs)
		{
			retDeferred.resolve();
		}
	};
	// fail callback, reject if ONE fails
	auto failCallback = [retDeferred]() mutable {
		retDeferred.reject();
	};
	// expand
	QDeferredDataBase::whenInternal(doneCallback, failCallback, t, rest...);
	// return deferred
	return retDeferred;
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
