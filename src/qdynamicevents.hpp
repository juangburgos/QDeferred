#ifndef QDYNAMICEVENTS_H
#define QDYNAMICEVENTS_H

#include <QExplicitlySharedDataPointer>
#include <QList>
#include <functional>
#include <QDebug>

#include "qdynamiceventsdata.hpp"

template<class ...Types>
class QDynamicEvents
{

public:
	// constructors
	QDynamicEvents();
	QDynamicEvents(const QDynamicEvents<Types...> &other);
	QDynamicEvents &operator=(const QDynamicEvents<Types...> &rhs);
	~QDynamicEvents();

	// consumer API

	// on method	
	QDynamicEventsHandle on(QString strEventName, std::function<void(Types(&...args))> callback);
	// off method (all callbacks registered to an specific event name)
	void off(QString strEventName);
	// off method (specific callback based on handle)
	void off(QDynamicEventsHandle evtHandle);
	// off method (all callbacks)
	void off();

	// provider API

	void trigger(QString strEventName, Types(&...args));

protected:
	QExplicitlySharedDataPointer<QDynamicEventsData<Types...>> m_data;

};

// alias for no argument types
using QDynEvts = QDynamicEvents<>;

template<class ...Types>
QDynamicEvents<Types...>::QDynamicEvents() : m_data(nullptr)
{
	m_data = QExplicitlySharedDataPointer<QDynamicEventsData<Types...>>(new QDynamicEventsData<Types...>());
}

template<class ...Types>
QDynamicEvents<Types...>::QDynamicEvents(const QDynamicEvents<Types...> &other) : m_data(other.m_data)
{
	m_data.reset();
	m_data = other.m_data;
}

template<class ...Types>
QDynamicEvents<Types...> & QDynamicEvents<Types...>::operator=(const QDynamicEvents<Types...> &rhs)
{
	if (this != &rhs) {
		m_data.reset();
		m_data.operator=(rhs.m_data);
	}
	return *this;
}

template<class ...Types>
QDynamicEvents<Types...>::~QDynamicEvents()
{
	m_data.reset();
}

template<class ...Types>
QDynamicEventsHandle QDynamicEvents<Types...>::on(QString strEventName, std::function<void(Types(&...args))> callback)
{
	// NOTE need to create the handle here in order to make it unique
	size_t evtHandle = (size_t)(&callback);
	return m_data->on(strEventName, evtHandle, callback);
}

template<class ...Types>
void QDynamicEvents<Types...>::off(QString strEventName)
{
	m_data->off(strEventName);
}

template<class ...Types>
void QDynamicEvents<Types...>::off(QDynamicEventsHandle evtHandle)
{
	m_data->off(evtHandle);
}

template<class ...Types>
void QDynamicEvents<Types...>::off()
{
	m_data->off();
}

template<class ...Types>
void QDynamicEvents<Types...>::trigger(QString strEventName, Types(&...args))
{
	// pass reference to this to at least have 1 reference until callbacks get executed
	m_data->trigger(*this, strEventName, args...);
}

#endif // QDYNAMICEVENTS_H