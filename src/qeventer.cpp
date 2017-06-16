#include "qeventer.h"

QEventer::QEventer()
{

}

QEventer::~QEventer()
{
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
