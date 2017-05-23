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

	// extra consume API (static)

	template <class ...OtherTypes, typename... Rest>
	static QDeferred<> when(QDeferred<OtherTypes...> t, Rest... rest);

	// wrapper provider API

	// resolve method
	void resolve(Types(&...args));
	// reject method
	void reject(Types(&...args));

	// internal API
	void zerop(std::function<void()> callback);

protected:
	QExplicitlySharedDataPointer<QDeferredData<Types...>> data;

private:
	// when internal method speacialization
	template <typename T>
	static void whenInternal(std::function<void()> callbackInternal, T t);
	// when internal method
	template <typename T, typename... Rest>
	static void whenInternal(std::function<void()> callbackInternal, T t, Rest... rest);
};

// alias for no argument types
using QDefer = QDeferred<>;

template<class ...Types>
QDeferred<Types...>::QDeferred() : data(nullptr)
{
	data = QExplicitlySharedDataPointer<QDeferredData<Types...>>(new QDeferredData<Types...>());
}

template<class ...Types>
QDeferred<Types...>::QDeferred(const QDeferred<Types...> &other) : data(other.data)
{
	data.reset();
	data = other.data;
}

template<class ...Types>
QDeferred<Types...> &QDeferred<Types...>::operator=(const QDeferred<Types...> &rhs)
{
	if (this != &rhs) {
		data.reset();
		data.operator=(rhs.data);
	}
	return *this;
}

template<class ...Types>
QDeferred<Types...>::~QDeferred()
{
	data.reset();
}

template<class ...Types>
QDeferredState QDeferred<Types...>::state()
{
	return data->state();
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::done(std::function<void(Types(&...args))> callback)
{
	data->done(callback);
	return *this;
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::fail(std::function<void(Types(&...args))> callback)
{
	data->fail(callback);
	return *this;
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::then(std::function<void(Types(&...args))> callback)
{
	data->then(callback);
	return *this;
}

template<class ...Types>
void QDeferred<Types...>::resolve(Types(&...args))
{
	data->resolve(args...);
}

template<class ...Types>
void QDeferred<Types...>::reject(Types(&...args))
{
	data->reject(args...);
}

template<class ...Types>
void QDeferred<Types...>::zerop(std::function<void()> callback)
{
	data->zerop(callback);
}

template<class ...Types> // NOTE : necessary to belong to class
template <typename T>
static void QDeferred<Types...>::whenInternal(std::function<void()> callbackInternal, T t)
{
	// add to zero params list
	t.zerop(callbackInternal);
}

template<class ...Types>
template <typename T, typename... Rest>
static void QDeferred<Types...>::whenInternal(std::function<void()> callbackInternal, T t, Rest... rest)
{
	// add to zero params list
	t.zerop(callbackInternal);
	// expand by recursion
	whenInternal(callbackInternal, rest...);
}

template<class ...Types>
template <class ...OtherTypes, typename... Rest>
static QDefer QDeferred<Types...>::when(QDeferred<OtherTypes...> t, Rest... rest)
{
	// setup necessary variables for expansion
	QDefer retDeferred;
	int    countArgs = sizeof...(Rest)+1;
	auto allThenFunc = [retDeferred, countArgs]() mutable {
		static int countThen = 0;
		countThen++;
		if (countThen == countArgs)
		{
			// TODO : resolve only if all resolve, reject if al least one rejected
			//        will need to make a zerop for done and a zerop for fail to be
			//        able to count individually
			/*
			https://api.jquery.com/jQuery.when/
			In the case where multiple Deferred objects are passed to jQuery.when(), the method returns the Promise from a 
			new "master" Deferred object that tracks the aggregate state of all the Deferreds it has been passed. 
			The method will resolve its master Deferred as soon as all the Deferreds resolve, or reject the master 
			Deferred as soon as one of the Deferreds is rejected.
			*/
			retDeferred.resolve();
		}
	};
	// expand
	whenInternal(allThenFunc, t, rest...);
	// return deferred
	return retDeferred;
}


#endif // QDEFERRED_H