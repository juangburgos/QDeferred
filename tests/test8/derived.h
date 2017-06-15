#ifndef DERIVED_H
#define DERIVED_H

#include <typeinfo>
#include <typeindex>

#include <QMap>
#include <QScopedPointer>
#include <QVariant>

#include <QDynamicEvents>

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

	// NOTE : with variadic templates compiles but VS intellisense fails and marks with ulgy red

	template<typename ...Types>
	QDynamicEventsHandle on(QString strEventName, std::function<void(Types(&...args))> callback);

private:
	bool    m_internalBool;
	int     m_internalInt;
	double  m_internalDouble;
	QString m_internalString;

	template<typename ...Types>
	void trigger(QString strEventName, Types(...args));

	// use combination of QMap and template function to emulate  variable templates
	// http://en.cppreference.com/w/cpp/language/variable_template
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
QDynamicEventsHandle Derived::on(QString strEventName, std::function<void(Types(&...args))> callback)
{
	return getEventer<Types...>().on(strEventName, callback);
}

template<typename ...Types>
void Derived::trigger(QString strEventName, Types(...args))
{
	return getEventer<Types...>().trigger(strEventName, args...);
}

#endif // DERIVED_H