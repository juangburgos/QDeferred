#include "derived.h"

Derived::Derived()
{
	m_internalBool   = false;
	m_internalInt    = 0;
	m_internalDouble = 0.0;
	m_internalString = "";
}

Derived::~Derived()
{
	m_mapEventers.clear();
}

void Derived::set_boolval(bool bVal)
{
	m_internalBool = bVal;
	trigger<bool>("change:boolval", m_internalBool);
	trigger<QString, QVariant>("change", "boolval", m_internalBool);
}

bool Derived::get_boolval()
{
	return m_internalBool;
}

void Derived::set_intval(int iVal)
{
	m_internalInt = iVal;
	trigger<int>("change:intval", m_internalInt);
	trigger<QString, QVariant>("change", "intval", m_internalInt);
}

int Derived::get_intval()
{
	return m_internalInt;
}

void Derived::set_doubleval(double dVal)
{
	m_internalDouble = dVal;
	trigger<double>("change:doubleval", m_internalDouble);
	trigger<QString, QVariant>("change", "doubleval", m_internalDouble);
}

double Derived::get_doubleval()
{
	return m_internalDouble;
}

void Derived::set_stringval(QString strVal)
{
	m_internalString = strVal;
	trigger<QString>("change:stringval", m_internalString);
	trigger<QString, QVariant>("change", "stringval", m_internalString);
}

QString Derived::get_stringval()
{
	return m_internalString;
}
