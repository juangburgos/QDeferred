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

	// wrapper provider API

	// resolve method
	void resolve(Types(&...args));
	// reject method
	void reject(Types(&...args));

protected:
	QExplicitlySharedDataPointer<QDeferredData<Types...>> data;

};

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


#endif // QDEFERRED_H