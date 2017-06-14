#ifndef DERIVED_H
#define DERIVED_H

#include <QVariant>

#include <QDynamicEvents>

class Derived :
	public QDynamicEvents<bool>    ,
	public QDynamicEvents<int>     ,
	public QDynamicEvents<double>  ,
	public QDynamicEvents<QString> ,
	public QDynamicEvents<QVariant>,
	public QDynamicEvents<QString, QVariant>
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
	//template<typename ...Types>
	//QDynamicEventsHandle on(QString strEventName, std::function<void(Types(&...args))> callback);
	template<typename T1>
	QDynamicEventsHandle on(QString strEventName, std::function<void(T1)> callback);
	template<typename T1, typename T2>
	QDynamicEventsHandle on(QString strEventName, std::function<void(T1, T2)> callback);

private:
	bool    m_internalBool;
	int     m_internalInt;
	double  m_internalDouble;
	QString m_internalString;

	template<typename ...Types>
	void trigger(QString strEventName, Types(...args));
};

//template<typename ...Types>
//QDynamicEventsHandle Derived::on(QString strEventName, std::function<void(Types(&...args))> callback)
//{
//	return QDynamicEvents<Types...>::on(strEventName, callback);
//}

template<typename T1>
QDynamicEventsHandle Derived::on(QString strEventName, std::function<void(T1)> callback)
{
	return QDynamicEvents<T1>::on(strEventName, callback);
}

template<typename T1, typename T2>
QDynamicEventsHandle Derived::on(QString strEventName, std::function<void(T1, T2)> callback)
{
	return QDynamicEvents<T1, T2>::on(strEventName, callback);
}

template<typename ...Types>
void Derived::trigger(QString strEventName, Types(...args))
{
	return QDynamicEvents<Types...>::trigger(strEventName, args...);
}

#endif // DERIVED_H