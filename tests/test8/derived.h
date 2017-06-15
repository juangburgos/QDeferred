#ifndef DERIVED_H
#define DERIVED_H

#include <QVariant>

#include <QDynamicEvents>

class Derived
{
public:
	Derived();

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

	template<typename ...Types>
	QDynamicEvents<Types...> getEventer();
};

template<typename ...Types>
QDynamicEvents<Types...> Derived::getEventer()
{
	// get only one eventer (static) for each types combination
	static QDynamicEvents<Types...> s_eventer;
	return s_eventer;
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