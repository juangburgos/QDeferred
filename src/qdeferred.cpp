#include "qdeferred.h"

QDeferred::QDeferred()
{

}

void QDeferred::done(std::function<void(QList<QVariant>)> callback)
{
	// append to done callbacks list
	m_doneList.append(callback);
}

void QDeferred::fail(std::function<void(QList<QVariant>)> callback)
{
	// append to fail callbacks list
	m_failList.append(callback);
}

QDeferred QDeferred::when(QList<QDeferred> listDeferred)
{
	// create deferred and resolve/fail it when all input deferreds have resolved/failed
	QDeferred retDeferred;
	int iResolveCount = 0;
	QList<QVariant> listAllArgs;
	// loop all input deferreds
	for (int i = 0; i < listDeferred.length(); i++)
	{
		listDeferred[i].done([&](QList<QVariant> listArgs) {
			listAllArgs.append(listArgs);
			iResolveCount++;
			// test if all resolved
			if (iResolveCount == listDeferred.length())
			{
				retDeferred.resolve(listAllArgs);
			}
		});
	}
	// return
	return retDeferred;
}

void QDeferred::resolve(QList<QVariant> &listArgs)
{
	// execute all done callbacks
	this->execute(this->m_doneList, listArgs);
}

void QDeferred::reject(QList<QVariant> &listArgs)
{
	// execute all fail callbacks
	this->execute(this->m_failList, listArgs);
}

void QDeferred::execute(QList<std::function<void(QList<QVariant>)>> &listCallbacks, QList<QVariant> &listArgs)
{
	// loop all callbacks
	for (int i = 0; i < listCallbacks.length(); i++)
	{
		// call each callback with arguments
		listCallbacks.at(i)(listArgs);
	}
}
