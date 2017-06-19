#include "qeventer.h"

QEventer::QEventer()
{
	// nothing to do here
}

QEventer::QEventer(const QEventer &other) :
	m_mapEventers(other.m_mapEventers)
{
	// nothing to do here
}

QEventer::~QEventer()
{
	qDeleteAll(m_mapEventers);
	m_mapEventers.clear();
}

void QEventer::off(QString strEventName)
{
	QMapIterator<std::type_index, QAbstractDynamicEvents*> i(m_mapEventers);
	while (i.hasNext()) {
		i.next();
		i.value()->off(strEventName);
	}
}

void QEventer::off(QDynamicEventsHandle evtHandle)
{
	QMapIterator<std::type_index, QAbstractDynamicEvents*> i(m_mapEventers);
	while (i.hasNext()) {
		i.next();
		i.value()->off(evtHandle);
	}
}

void QEventer::off()
{
	QMapIterator<std::type_index, QAbstractDynamicEvents*> i(m_mapEventers);
	while (i.hasNext()) {
		i.next();
		i.value()->off();
	}
}
