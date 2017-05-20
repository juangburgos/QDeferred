#ifndef QDEFERRED_H
#define QDEFERRED_H

#include <QList>
#include <functional>

template<class T>
class QDeferredData
{
	
public:
	// consumer API

	// done method	
	void done(std::function<void(T)> callback);
	// fail method
	void fail(std::function<void(T)> callback);
	//// when method
	//template <typename T>
	//static QDeferredData when(QList<QDeferredData> listDeferred);

	// provider API

	// resolve method
	void resolve(T &cargs);
	// reject method
	void reject(T &cargs);

private:
	// members
	QList< std::function<void(T)> > m_doneList;
	QList< std::function<void(T)> > m_failList;
	// methods
	void execute(QList< std::function<void(T)> > &listCallbacks, T &cargs);

};

template<class T>
void QDeferredData<T>::done(std::function<void(T)> callback)
{
	// append to fail callbacks list
	m_doneList.append(callback);
}

template<class T>
void QDeferredData<T>::fail(std::function<void(T)> callback)
{
	// append to done callbacks list
	m_failList.append(callback);
}

template<class T>
void QDeferredData<T>::resolve(T &cargs)
{
	// execute all done callbacks
	this->execute(this->m_doneList, cargs);
}

template<class T>
void QDeferredData<T>::reject(T &cargs)
{
	// execute all fail callbacks
	this->execute(this->m_failList, cargs);
}

template<class T>
void QDeferredData<T>::execute(QList< std::function<void(T)> > &listCallbacks, T &cargs)
{
	// loop all callbacks
	for (int i = 0; i < listCallbacks.length(); i++)
	{
		// call each callback with arguments
		listCallbacks.at(i)(cargs);
	}
}

// ---------------------------------------------
//	// create deferred and resolve/fail it when all input deferreds have resolved/failed
//	QDeferredData retDeferred;
//	int iResolveCount = 0;
//	// loop all input deferreds
//	for (int i = 0; i < listDeferred.length(); i++)
//	{
//		listDeferred[i].done([&](T args) {
//			iResolveCount++;
//			// test if all resolved
//			if (iResolveCount == listDeferred.length())
//			{
//				retDeferred.resolve();
//			}
//		});
//	}
//	// return
//	return retDeferred;

#endif // QDEFERRED_H