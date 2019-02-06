#ifndef QEVENTER_H
#define QEVENTER_H

#include <typeinfo>
#include <typeindex>

#include <QMap>
#include <QScopedPointer>
#include <QVariant>

#include <QDynamicEvents>

// TODO : pass on the const & idiom deeper into implementation

// base class to inherit from to add all eventer's functionality
class QEventer
{
public:
	QEventer();
	~QEventer();
	QEventer(const QEventer &other);

	// consumer API

	// on method
	// NOTE : inline below are useless, only to avoid intellisense ugly read
	template<typename ...Types, typename T1, typename T2>
	QDynamicEventsHandle on(const QString &strEventName, 
		                    const T1      &callback, 
		                    const T2      &filter = nullptr, 
		                    const Qt::ConnectionType &connection = Qt::AutoConnection) {
		return onAlias<Types...>(strEventName, callback, filter, connection);
	};
	// once method	
	template<typename ...Types, typename T1, typename T2>
	QDynamicEventsHandle once(const QString &strEventName, 
		                      const T1      &callback, 
		                      const T2      &filter = nullptr, 
		                      const Qt::ConnectionType &connection = Qt::AutoConnection) {
		return onceAlias<Types...>(strEventName, callback, filter, connection);
	};
	// off method (all callbacks registered to an specific event name)
	void off(const QString &strEventName);
	// off method (specific callback based on handle)
	void off(const QDynamicEventsHandle &evtHandle);
	// off method (all callbacks)
	void off();

	// provider API

	// trigger event method
	template<typename ...Types>
	void trigger(QString strEventName, Types(...args));

protected:
	// without alias would work, but annoying intellisense appears 
	template<typename ...Types>
	QDynamicEventsHandle onAlias(QString strEventName, std::function<void(Types(&...args))> callback, std::function<bool(Types(&...args))> filter = nullptr, Qt::ConnectionType connection = Qt::AutoConnection);
	template<typename ...Types>
	QDynamicEventsHandle onceAlias(QString strEventName, std::function<void(Types(&...args))> callback, std::function<bool(Types(&...args))> filter = nullptr, Qt::ConnectionType connection = Qt::AutoConnection);

	/*
	use combination of QMap and template function to emulate variable templates
	http://en.cppreference.com/w/cpp/language/variable_template
	https://stackoverflow.com/questions/37912378/variable-templates-only-available-with-c14
	http://en.cppreference.com/w/cpp/types/type_index
	*/
	QMap<std::type_index, QAbstractDynamicEvents*> m_mapEventers;
	template<typename ...Types>
	QDynamicEvents<Types...> getEventer();
};

template<typename ...Types>
QDynamicEvents<Types...> QEventer::getEventer()
{
	// create once eventer for each Types... (combination of types) used in code
	if (!m_mapEventers.contains(std::type_index(typeid(QDynamicEvents<Types...>))))
	{
		m_mapEventers[std::type_index(typeid(QDynamicEvents<Types...>))] = new QDynamicEvents<Types...>;
		//qDebug() << "[INFO] Created eventer for type " << typeid(QDynamicEvents<Types...>).name();
	}
	// dereference and return
	return *(static_cast<QDynamicEvents<Types...> *>(m_mapEventers[std::type_index(typeid(QDynamicEvents<Types...>))]));
}

template<typename ...Types>
QDynamicEventsHandle QEventer::onAlias(QString strEventName, std::function<void(Types(&...args))> callback, std::function<bool(Types(&...args))> filter/* = nullptr*/, Qt::ConnectionType connection/* = Qt::AutoConnection*/)
{
	return getEventer<Types...>().on(strEventName, callback, filter, connection);
}

template<typename ...Types>
QDynamicEventsHandle QEventer::onceAlias(QString strEventName, std::function<void(Types(&...args))> callback, std::function<bool(Types(&...args))> filter/* = nullptr*/, Qt::ConnectionType connection/* = Qt::AutoConnection*/)
{
	return getEventer<Types...>().once(strEventName, callback, filter, connection);
}


template<typename ...Types>
void QEventer::trigger(QString strEventName, Types(...args))
{
	return getEventer<Types...>().trigger(strEventName, args...);
}

#endif