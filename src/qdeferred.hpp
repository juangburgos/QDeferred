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
	QDeferred(const QDeferred<Types...> &);
	QDeferred &operator=(const QDeferred<Types...> &);
	~QDeferred();

	// wrapper consumer API (with chaning)

	// get state method
	QDeferredState state();

	// done method	
	QDeferred<Types...> done(std::function<void(Types (&...args))> callback); // by copy would be <Types... arg>
	// fail method
	QDeferred<Types...> fail(std::function<void(Types(&...args))> callback);
	// then method
	QDeferred<Types...> then(std::function<void(Types(&...args))> callback);
	// progress method
	QDeferred<Types...> progress(std::function<void(Types(&...args))> callback);

	// extra consume API (static)

	template <class ...OtherTypes, typename... Rest>
	static QDeferred<> when(QDeferred<OtherTypes...> t, Rest... rest);

	// wrapper provider API

	// resolve method
	void resolve(Types(&...args));
	// reject method
	void reject(Types(&...args));
	// notify method
	void notify(Types(&...args));

protected:
	QExplicitlySharedDataPointer<QDeferredData<Types...>> m_data;

private:

	// friend class
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
QDeferredState QDeferred<Types...>::state()
{
	return m_data->state();
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::done(std::function<void(Types(&...args))> callback)
{
	m_data->done(callback);
	return *this;
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::fail(std::function<void(Types(&...args))> callback)
{
	m_data->fail(callback);
	return *this;
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::then(std::function<void(Types(&...args))> callback)
{
	m_data->then(callback);
	return *this;
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::progress(std::function<void(Types(&...args))> callback)
{
	m_data->progress(callback);
	return *this;
}

template<class ...Types>
void QDeferred<Types...>::resolve(Types(&...args))
{
	// pass reference to this to at least have 1 reference until callbacks get executed
	m_data->resolve(*this, args...);
}

template<class ...Types>
void QDeferred<Types...>::reject(Types(&...args))
{
	// pass reference to this to at least have 1 reference until callbacks get executed
	m_data->reject(*this, args...);
}

template<class ...Types>
void QDeferred<Types...>::notify(Types(&...args))
{
	// pass reference to this to at least have 1 reference until callbacks get executed
	m_data->notify(*this, args...);
}

template<class ...Types>
void QDeferred<Types...>::doneZero(std::function<void()> callback)
{
	m_data->doneZero(callback);
}

template<class ...Types>
void QDeferred<Types...>::failZero(std::function<void()> callback)
{
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
static QDefer QDeferred<Types...>::when(QDeferred<OtherTypes...> t, Rest... rest)
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