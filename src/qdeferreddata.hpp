#ifndef QDEFERREDDATA_H
#define QDEFERREDDATA_H

#include <QSharedData>
#include <QList>
#include <functional>

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

private:
	// members
	QList< std::function<void(Types(&...args))> > m_doneList;
	QList< std::function<void(Types(&...args))> > m_failList;
	QList< std::function<void(Types(&...args))> > m_thenList;
	QList< std::function<void()> > m_doneZeroList;
	QList< std::function<void()> > m_failZeroList;
	QDeferredState m_state;
	std::function<void(std::function<void(Types(&...args))>)> m_finishedFunction;
	// methods
	void execute(QList< std::function<void(Types(&...args))> > &listCallbacks, Types(&...args));

};



template<class ...Types>
QDeferredData<Types...>::QDeferredData()
{
	m_state = QDeferredState::PENDING;
}

template<class ...Types>
QDeferredData<Types...>::QDeferredData(const QDeferredData &other) : QSharedData(other),
m_doneList(other.m_doneList),
m_failList(other.m_failList),
m_thenList(other.m_thenList),
m_doneZeroList(other.m_doneZeroList),
m_failZeroList(other.m_failZeroList),
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
	// append to done callbacks list
	m_doneList.append(callback);
	// call it inmediatly if already resolved
	if (m_state == QDeferredState::RESOLVED)
	{
		m_finishedFunction(callback);
	}
}

template<class ...Types>
void QDeferredData<Types...>::fail(std::function<void(Types(&...args))> callback)
{
	// append to fail callbacks list
	m_failList.append(callback);
	// call it inmediatly if already rejected
	if (m_state == QDeferredState::REJECTED)
	{
		m_finishedFunction(callback);
	}
}

template<class ...Types>
void QDeferredData<Types...>::then(std::function<void(Types(&...args))> callback)
{
	// append to then callbacks list
	m_thenList.append(callback);
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
	// execute all done callbacks
	this->execute(this->m_doneList, args...);
	// execute all then callbacks
	this->execute(this->m_thenList, args...);
	// loop all done zero callbacks
	for (int i = 0; i < m_doneZeroList.length(); i++)
	{
		// call each callback
		m_doneZeroList.at(i)();
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
	// execute all fail callbacks
	this->execute(this->m_failList, args...);
	// execute all then callbacks
	this->execute(this->m_thenList, args...);
	// loop all fail zero callbacks
	for (int i = 0; i < m_failZeroList.length(); i++)
	{
		// call each callback
		m_failZeroList.at(i)();
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
	// append to done zero callbacks list
	m_doneZeroList.append(callback);
	// call it inmediatly if already resolved
	if (m_state == QDeferredState::RESOLVED)
	{
		callback();
	}
}

template<class ...Types>
void QDeferredData<Types...>::failZero(std::function<void()> callback)
{
	// append to fail zero callbacks list
	m_failZeroList.append(callback);
	// call it inmediatly if already rejected
	if (m_state == QDeferredState::REJECTED)
	{
		callback();
	}
}

#endif // QDEFERREDDATA_H