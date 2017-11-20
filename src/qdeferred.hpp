#ifndef QDEFERRED_H
#define QDEFERRED_H

#include <QExplicitlySharedDataPointer>
#include <QList>
#include <functional>
#include <QDebug>

#include "qdeferreddata.hpp"

template<class ...Types>
class QDeferred
{
	
public:
	// constructors
	QDeferred();
	QDeferred(const QDeferred<Types...> &other);
	QDeferred &operator=(const QDeferred<Types...> &rhs);
	~QDeferred();

	// wrapper consumer API (with chaning)

	// get state method
	QDeferredState state() const;

	// done method	
	QDeferred<Types...> done(std::function<void(Types (&...args))> callback); // by copy would be <Types... arg>
	// fail method
	QDeferred<Types...> fail(std::function<void(Types(&...args))> callback);
	// then method
	QDeferred<Types...> then(std::function<void(Types(&...args))> doneCallback, 
		                     std::function<void(Types(&...args))> failCallback     = std::function<void(Types(&...args))>(),
		                     std::function<void(Types(&...args))> progressCallback = std::function<void(Types(&...args))>());
	// progress method
	QDeferred<Types...> progress(std::function<void(Types(&...args))> callback);

	// wrapper consumer visual studio debug API (native visualizations)
#if defined(QT_DEBUG) && defined(Q_OS_WIN) && defined(JS_DEBUG)
	// debug done method	
	QDeferred<Types...> doneVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, std::function<void(Types(&...args))> callback);
	// debug fail method
	QDeferred<Types...> failVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, std::function<void(Types(&...args))> callback);
	// TODO : fix
	// debug then method
	QDeferred<Types...> thenVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, std::function<void(Types(&...args))> callback);
	// debug progress method
	QDeferred<Types...> progressVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, std::function<void(Types(&...args))> callback);
#endif

	// extra consume API (static)

	template <class ...OtherTypes, typename... Rest>
	static QDeferred<> when(QDeferred<OtherTypes...> t, Rest... rest);

	// wrapper provider API

	// resolve method
	void resolve(Types(&...args));
	// reject method
	void reject(Types(&...args));
	// notify method
	void notify(Types(&...args));

	// wrapper provider visual studio debug API (native visualizations)
#if defined(QT_DEBUG) && defined(Q_OS_WIN) && defined(JS_DEBUG)
	// debug resolve method
	void resolveVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, Types(&...args));
	// debug reject method
	void rejectVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, Types(&...args));
	// debug notify method
	void notifyVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, Types(&...args));
#endif

protected:
	QExplicitlySharedDataPointer<QDeferredData<Types...>> m_data;

private:

	// friend class
	friend class QDeferredDataBase;

	// internal methods

	// get when count method
	int  getWhenCount();
	// set when count method
	void setWhenCount(int whenCount);

	// done method with zero arguments
	void doneZero(std::function<void()> callback);
	// fail method with zero arguments
	void failZero(std::function<void()> callback);

	// debug helpers for visual studio debug API (native visualizations)
#if defined(QT_DEBUG) && defined(Q_OS_WIN) && defined(JS_DEBUG)
	QString createVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd);
#endif
};

// alias for no argument types
using QDefer = QDeferred<>;

#if defined(QT_DEBUG) && defined(Q_OS_WIN) && defined(JS_DEBUG)
template<class ...Types>
inline QString QDeferred<Types...>::createVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd)
{
	// get thread address
	std::stringstream stream;
	stream << std::hex << (size_t)thnd;
	// clean lambda function name
	QString strFunc = QString("%1").arg(function);
	if (strFunc.contains("lambda_"))
	{
		//QString &QString::remove(int position, int n)
		int iPos = strFunc.indexOf("lambda_") + 6;
		int iN   = strFunc.indexOf(">") - iPos;
		strFunc  = strFunc.remove(iPos, iN);
	}
	return QString("File: %1, Func: %2, Line: %3, Thd: %4").arg(file).arg(strFunc).arg(line).arg(QString::fromStdString(stream.str()));
}
#endif

template<class ...Types>
QDeferred<Types...>::QDeferred() : m_data(nullptr)
{
	m_data = QExplicitlySharedDataPointer<QDeferredData<Types...>>(new QDeferredData<Types...>());
}

template<class ...Types>
QDeferred<Types...>::QDeferred(const QDeferred<Types...> &other) : m_data(other.m_data)
{
	m_data.reset();
	m_data = other.m_data;
}

template<class ...Types>
QDeferred<Types...> &QDeferred<Types...>::operator=(const QDeferred<Types...> &rhs)
{
	if (this != &rhs) {
		m_data.reset();
		m_data.operator=(rhs.m_data);
	}
	return *this;
}

template<class ...Types>
QDeferred<Types...>::~QDeferred()
{
	m_data.reset();
}

template<class ...Types>
QDeferredState QDeferred<Types...>::state() const
{
	return m_data->state();
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::done(std::function<void(Types(&...args))> callback)
{
	m_data->done(callback);
	return *this;
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::fail(std::function<void(Types(&...args))> callback)
{
	m_data->fail(callback);
	return *this;
}

// TODO : fix
//template<class ...Types>
//QDeferred<Types...> QDeferred<Types...>::then(std::function<void(Types(&...args))> callback)
//{
//	m_data->then(callback);
//	return *this;
//}
template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::then(
	std::function<void(Types(&...args))> doneCallback, 
	std::function<void(Types(&...args))> failCallback     /*= std::function<void(Types(&...args))>()*/, 
	std::function<void(Types(&...args))> progressCallback /*= std::function<void(Types(&...args))>()*/)
{
	// done callback is mandatory
	m_data->done(doneCallback);
	// check if valid
	if (failCallback)
	{
		m_data->fail(failCallback);
	}
	// check if valid
	if (progressCallback)
	{
		m_data->progress(progressCallback);
	}
	return *this;
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::progress(std::function<void(Types(&...args))> callback)
{
	m_data->progress(callback);
	return *this;
}

#if defined(QT_DEBUG) && defined(Q_OS_WIN) && defined(JS_DEBUG)
template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::doneVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, std::function<void(Types(&...args))> callback)
{
	QString strDbgInfo = createVsDbg_Impl(file, function, line, thnd);
	// call debug version
	m_data->doneVsDbg_Impl(strDbgInfo, callback);
	return *this;
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::failVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, std::function<void(Types(&...args))> callback)
{
	QString strDbgInfo = createVsDbg_Impl(file, function, line, thnd);
	// call debug version
	m_data->failVsDbg_Impl(strDbgInfo, callback);
	return *this;
}

// TODO : fix
template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::thenVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, std::function<void(Types(&...args))> callback)
{
	QString strDbgInfo = createVsDbg_Impl(file, function, line, thnd);
	// call debug version
	m_data->thenVsDbg_Impl(strDbgInfo, callback);
	return *this;
}

template<class ...Types>
QDeferred<Types...> QDeferred<Types...>::progressVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, std::function<void(Types(&...args))> callback)
{
	QString strDbgInfo = createVsDbg_Impl(file, function, line, thnd);
	// call debug version
	m_data->progressVsDbg_Impl(strDbgInfo, callback);
	return *this;
}
#endif // defined(QT_DEBUG) && defined(Q_OS_WIN)

template<class ...Types>
void QDeferred<Types...>::resolve(Types(&...args))
{
	// pass reference to this to at least have 1 reference until callbacks get executed
	m_data->resolve(*this, args...);
}

template<class ...Types>
void QDeferred<Types...>::reject(Types(&...args))
{
	// pass reference to this to at least have 1 reference until callbacks get executed
	m_data->reject(*this, args...);
}

template<class ...Types>
void QDeferred<Types...>::notify(Types(&...args))
{
	// pass reference to this to at least have 1 reference until callbacks get executed
	m_data->notify(*this, args...);
}

#if defined(QT_DEBUG) && defined(Q_OS_WIN) && defined(JS_DEBUG)
#include <sstream>

template<class ...Types>
void QDeferred<Types...>::resolveVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, Types(&...args))
{
	QString strDbgInfo = createVsDbg_Impl(file, function, line, thnd);
	// call debug version
	m_data->resolveVsDbg_Impl(strDbgInfo, *this, args...);
}

template<class ...Types>
void QDeferred<Types...>::rejectVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, Types(&...args))
{
	QString strDbgInfo = createVsDbg_Impl(file, function, line, thnd);
	// call debug version
	m_data->rejectVsDbg_Impl(strDbgInfo, *this, args...);
}

template<class ...Types>
void QDeferred<Types...>::notifyVsDbg_Impl(const char *file, const char *function, const int line, QThread * thnd, Types(&...args))
{
	QString strDbgInfo = createVsDbg_Impl(file, function, line, thnd);
	// call debug version
	m_data->notifyVsDbg_Impl(strDbgInfo, *this, args...);
}
#endif // defined(QT_DEBUG) && defined(Q_OS_WIN)

template<class ...Types>
void QDeferred<Types...>::doneZero(std::function<void()> callback)
{
	m_data->doneZero(callback);
}

template<class ...Types>
void QDeferred<Types...>::failZero(std::function<void()> callback)
{
	m_data->failZero(callback);
}

/*
https://api.jquery.com/jQuery.when/
In the case where multiple Deferred objects are passed to jQuery.when(), the method returns the Promise from a
new "master" Deferred object that tracks the aggregate state of all the Deferreds it has been passed.
The method will resolve its master Deferred as soon as all the Deferreds resolve, or reject the master
Deferred as soon as one of the Deferreds is rejected.
*/
template<class ...Types>
template<class ...OtherTypes, class... Rest>
QDefer QDeferred<Types...>::when(QDeferred<OtherTypes...> t, Rest... rest)
{
	// setup necessary variables for expansion
	QDefer retDeferred;
	int    countArgs = sizeof...(Rest)+1;
	// done callback, resolve if ALL done
	auto doneCallback = [retDeferred, countArgs]() mutable {
		retDeferred.setWhenCount(retDeferred.getWhenCount() + 1); // whenCount++
		int whenCount = retDeferred.getWhenCount();
		if (whenCount == countArgs)
		{
			retDeferred.resolve();
		}
	};
	// fail callback, reject if ONE fails
	auto failCallback = [retDeferred]() mutable {
		retDeferred.reject();
	};
	// expand
	QDeferredDataBase::whenInternal(doneCallback, failCallback, t, rest...);
	// return deferred
	return retDeferred;
}

template<class ...Types>
void QDeferred<Types...>::setWhenCount(int whenCount)
{
	m_data->m_whenCount = whenCount;
}

template<class ...Types>
int QDeferred<Types...>::getWhenCount()
{
	return m_data->m_whenCount;
}

#if defined(QT_DEBUG) && defined(Q_OS_WIN) && defined(JS_DEBUG)
#define doneVsDbg(...)     doneVsDbg_Impl    (__FILE__,__FUNCTION__,__LINE__, QThread::currentThread(), __VA_ARGS__)
#define failVsDbg(...)     failVsDbg_Impl    (__FILE__,__FUNCTION__,__LINE__, QThread::currentThread(), __VA_ARGS__)
#define thenVsDbg(...)     thenVsDbg_Impl    (__FILE__,__FUNCTION__,__LINE__, QThread::currentThread(), __VA_ARGS__)
#define progressVsDbg(...) progressVsDbg_Impl(__FILE__,__FUNCTION__,__LINE__, QThread::currentThread(), __VA_ARGS__)
#define resolveVsDbg(...)  resolveVsDbg_Impl (__FILE__,__FUNCTION__,__LINE__, QThread::currentThread(), __VA_ARGS__)
#define rejectVsDbg(...)   rejectVsDbg_Impl  (__FILE__,__FUNCTION__,__LINE__, QThread::currentThread(), __VA_ARGS__)
#define notifyVsDbg(...)   notifyVsDbg_Impl  (__FILE__,__FUNCTION__,__LINE__, QThread::currentThread(), __VA_ARGS__)
#else
#define doneVsDbg(...)     done(__VA_ARGS__)    
#define failVsDbg(...)     fail(__VA_ARGS__)    
#define thenVsDbg(...)     then(__VA_ARGS__)    
#define progressVsDbg(...) progress(__VA_ARGS__)
#define resolveVsDbg(...)  resolve(__VA_ARGS__) 
#define rejectVsDbg(...)   reject(__VA_ARGS__)  
#define notifyVsDbg(...)   notify(__VA_ARGS__)  
#endif

#endif // QDEFERRED_H
