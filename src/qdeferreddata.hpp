#ifndef QDEFERRED_H
#define QDEFERRED_H

#include <QList>
#include <functional>

template<class ...Types>
class QDeferredData
{
	
public:
	// consumer API

	// done method	
	void done(std::function<void(Types (&...args))> callback); // by copy would be <Types... arg>
	// fail method
	void fail(std::function<void(Types(&...args))> callback);
	//// when method
	//template <typename T>
	//static QDeferredData when(QList<QDeferredData> listDeferred);

	// provider API

	// resolve method
	void resolve(Types(&...args));
	// reject method
	void reject(Types(&...args));

private:
	// members
	QList< std::function<void(Types(&...args))> > m_doneList;
	QList< std::function<void(Types(&...args))> > m_failList;
	// methods
	void execute(QList< std::function<void(Types(&...args))> > &listCallbacks, Types(&...args));

};

template<class ...Types>
void QDeferredData<Types...>::done(std::function<void(Types(&...args))> callback)
{
	// append to fail callbacks list
	m_doneList.append(callback);
}

template<class ...Types>
void QDeferredData<Types...>::fail(std::function<void(Types(&...args))> callback)
{
	// append to done callbacks list
	m_failList.append(callback);
}

template<class ...Types>
void QDeferredData<Types...>::resolve(Types(&...args))
{
	// execute all done callbacks
	this->execute(this->m_doneList, args...);
}

template<class ...Types>
void QDeferredData<Types...>::reject(Types(&...args))
{
	// execute all fail callbacks
	this->execute(this->m_failList, args...);
}

template<class ...Types>
void QDeferredData<Types...>::execute(QList< std::function<void(Types(&...args))> > &listCallbacks, Types(&...args))
{
	// loop all callbacks
	for (int i = 0; i < listCallbacks.length(); i++)
	{
		// call each callback with arguments
		listCallbacks.at(i)(args...);
	}
}


/*



template<class T>
void QDeferredData<T>::execute(QList< std::function<void(T)> > &listCallbacks, T &cargs)
{

}

*/


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