#include "qdeferreddata.hpp"

#include <QDebug>

// define static members and methods of base class

int    QDeferredDataBase::s_createCount = 0;

QMutex QDeferredDataBase::s_mutex;

QMap< QThread *, QDeferredProxyObject * > QDeferredDataBase::s_threadMap;

QDeferredProxyObject * QDeferredDataBase::getObjectForThread(QThread * p_currThd)
{
	// lock multithread access
	QDeferredDataBase::s_mutex.lock();
	// if not in list then...
	if (!s_threadMap.contains(p_currThd))
	{
		// subscribe to finish
		QObject::connect(p_currThd, &QThread::finished, [p_currThd]() {
			//qDebug() << "[INFO] Thread " << p_currThd << " finished from thread " << QThread::currentThread();
			// process thread events
			//QCoreApplication::processEvents();
			// if finished, remove
			auto p_objToDelete = QDeferredDataBase::s_threadMap.take(p_currThd);
			// mark the object for deletion
			p_objToDelete->deleteLater();
			//// [DEBUG]
			//QDeferredDataBase::s_createCount--;
			//qDebug() << "[INFO] Number of QDeferredProxyObject = " << QDeferredDataBase::s_createCount;
		});
		// add to all maps
		s_threadMap[p_currThd] = new QDeferredProxyObject;
		//// [DEBUG]
		//QDeferredDataBase::s_createCount++;
		//qDebug() << "[INFO] Number of QDeferredProxyObject = " << QDeferredDataBase::s_createCount;
	}
	// unlock multithread access
	QDeferredDataBase::s_mutex.unlock();
	// return
	return s_threadMap[p_currThd];
}

