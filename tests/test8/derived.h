#ifndef DERIVED_H
#define DERIVED_H

#include <typeinfo>
#include <typeindex>

#include <QMap>
#include <QScopedPointer>
#include <QVariant>

#include <QDynamicEvents>

// TODO : make a base class in order to add functionality by inheriting

class Derived
{
public:
	Derived();
	~Derived();

	void    set_boolval(bool bVal);
	bool    get_boolval();

	void    set_intval(int iVal);
	int     get_intval();

	void    set_doubleval(double dVal);
	double  get_doubleval();

	void    set_stringval(QString strVal);
	QString get_stringval();

	// NOTE : inline below are useless, only to avoid intellisense ugly read
	template<typename ...Types, typename T>
	inline QDynamicEventsHandle on(QString strEventName, T callback) {
		return onAlias<Types...>(strEventName, callback);
	};

private:
	bool    m_internalBool;
	int     m_internalInt;
	double  m_internalDouble;
	QString m_internalString;

	template<typename ...Types>
	void trigger(QString strEventName, Types(...args));

	// without alias would work, but annoying intellisense appears 
	template<typename ...Types>
	QDynamicEventsHandle onAlias(QString strEventName, std::function<void(Types(&...args))> callback);

	/*
	use combination of QMap and template function to emulate  variable templates
	http://en.cppreference.com/w/cpp/language/variable_template
	https://stackoverflow.com/questions/37912378/variable-templates-only-available-with-c14
	http://en.cppreference.com/w/cpp/types/type_index
	*/
	QMap<std::type_index, QAbstractDynamicEvents*> m_mapEventers;
	template<typename ...Types>
	QDynamicEvents<Types...> getEventer();
};

template<typename ...Types>
QDynamicEvents<Types...> Derived::getEventer()
{
	// create once eventer for each Types... (combination of types) used in code
	if ( !m_mapEventers.contains( std::type_index(typeid( QDynamicEvents<Types...> )) ) )
	{
		m_mapEventers[std::type_index(typeid(QDynamicEvents<Types...>))] = new QDynamicEvents<Types...>;
		qDebug() << "[INFO] Created eventer for type " << typeid(QDynamicEvents<Types...>).name();
	}
	return * (static_cast<QDynamicEvents<Types...> *>( m_mapEventers[std::type_index(typeid(QDynamicEvents<Types...>))] ));
}

template<typename ...Types>
QDynamicEventsHandle Derived::onAlias(QString strEventName, std::function<void(Types(&...args))> callback)
{
	return getEventer<Types...>().on(strEventName, callback);
}

template<typename ...Types>
void Derived::trigger(QString strEventName, Types(...args))
{
	return getEventer<Types...>().trigger(strEventName, args...);
}

#endif // DERIVED_H