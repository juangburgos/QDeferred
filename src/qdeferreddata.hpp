#ifndef QDEFERREDDATA_H
#define QDEFERREDDATA_H

#include <QSharedData>
#include <QList>
#include <functional>

template<class ...Types>
class QDeferredData : public QSharedData
{
	
public:
	// constructors
	QDeferredData();
	QDeferredData(const QDeferredData &other);

	// consumer API

	// done method	
	void done(std::function<void(Types (&...args))> callback); // by copy would be <Types... arg>
	// fail method
	void fail(std::function<void(Types(&...args))> callback);

	// provider API

	// resolve method
	void resolve(Types(&...args));
	// reject method
	void reject(Types(&...args));

private:
	// members
	QList< std::function<void(Types(&...args))> > m_doneList;
	QList< std::function<void(Types(&...args))> > m_failList;
	// methods
	void execute(QList< std::function<void(Types(&...args))> > &listCallbacks, Types(&...args));

};

template<class ...Types>
QDeferredData<Types...>::QDeferredData()
{

}

template<class ...Types>
QDeferredData<Types...>::QDeferredData(const QDeferredData &other) : QSharedData(other),
m_doneList(other.m_doneList),
m_failList(other.m_failList)
{
	// nothing to do here
}

template<class ...Types>
void QDeferredData<Types...>::done(std::function<void(Types(&...args))> callback)
{
	// append to fail callbacks list
	m_doneList.append(callback);
}

template<class ...Types>
void QDeferredData<Types...>::fail(std::function<void(Types(&...args))> callback)
{
	// append to done callbacks list
	m_failList.append(callback);
}

template<class ...Types>
void QDeferredData<Types...>::resolve(Types(&...args))
{
	// execute all done callbacks
	this->execute(this->m_doneList, args...);
}

template<class ...Types>
void QDeferredData<Types...>::reject(Types(&...args))
{
	// execute all fail callbacks
	this->execute(this->m_failList, args...);
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

#endif // QDEFERREDDATA_H