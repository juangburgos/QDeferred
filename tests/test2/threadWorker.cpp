#include "threadworker.h"

#include <QTimer>
#include <QDebug>

// THREADWORKEREVENT -------------------------------------------------

ThreadWorkerEvent::ThreadWorkerEvent() : QEvent(QWORKERPROXY_EVENT_TYPE)
{
	// nothing to do here
}

// THREADWORKER -----------------------------------------------------

ThreadWorker::ThreadWorker() : QObject(nullptr)
{

}

bool ThreadWorker::event(QEvent * ev)
{
	if (ev->type() == QWORKERPROXY_EVENT_TYPE) {
		// call function
		static_cast<ThreadWorkerEvent*>(ev)->m_eventFunc();
		// return event processed
		return true;
	}
	// Call base implementation (make sure the rest of events are handled)
	return QObject::event(ev);
}

// THREADCONTROLLER -----------------------------------------------

ThreadController::ThreadController()
{
	mp_worker = new ThreadWorker;
	mp_worker->moveToThread(&m_workerThread);
	connect(&m_workerThread, &QThread::finished, mp_worker, &QObject::deleteLater);
	m_workerThread.start();
}

ThreadController::~ThreadController()
{
	m_workerThread.quit();
	m_workerThread.wait();
}

void ThreadController::doWorkOnFirstDeferred(QDefer deferred1, QDeferred<int, double> deferred2)
{
	ThreadWorkerEvent * p_Evt = new ThreadWorkerEvent;
	p_Evt->m_eventFunc = [deferred1, deferred2]() mutable {
		// print id
		auto p_thread = QThread::currentThread();
		qDebug() << "[INFO] 1 thread id = " << p_thread;
		// set done
		deferred1.done([p_thread]() {
			qDebug() << "[INFO] DEF1::Callback defined in 1 thread " << p_thread << ", exec in thread " << QThread::currentThread();
		});
		deferred2.done([p_thread](int i, double d) {
			Q_UNUSED(i)
			Q_UNUSED(d)
			qDebug() << "[INFO] DEF2::Callback defined in 1 thread " << p_thread << ", exec in thread " << QThread::currentThread();
		});
		QDefer::when(deferred1, deferred2).done([p_thread]() {
			qDebug() << "[INFO] WHEN::Callback defined in 1 thread " << p_thread << ", exec in thread " << QThread::currentThread();
		});
		// set resolve timer
		QTimer::singleShot(1000, [deferred1]() mutable {
			qDebug() << "[INFO] DEF1::Resolved in 1 thread ********" << QThread::currentThread() << "********";
			deferred1.resolve();
		});
	};
	// post event for object with correct thread affinity
	QCoreApplication::postEvent(mp_worker, p_Evt);
}

void ThreadController::doWorkOnSecondDeferred(QDefer deferred1, QDeferred<int, double> deferred2)
{
	ThreadWorkerEvent * p_Evt = new ThreadWorkerEvent;
	p_Evt->m_eventFunc = [deferred1, deferred2]() mutable {
		// print id
		auto p_thread = QThread::currentThread();
		qDebug() << "[INFO] 2 thread id = " << p_thread;
		// set done
		deferred2.done([p_thread](int i, double d) {
			Q_UNUSED(i)
			Q_UNUSED(d)
			qDebug() << "[INFO] DEF2::Callback defined in 2 thread " << p_thread << ", exec in thread " << QThread::currentThread();
		});
		deferred1.done([p_thread]() {
			qDebug() << "[INFO] DEF1::Callback defined in 2 thread " << p_thread << ", exec in thread " << QThread::currentThread();
		});
		QDefer::when(deferred1, deferred2).done([p_thread]() {
			qDebug() << "[INFO] WHEN::Callback defined in 2 thread " << p_thread << ", exec in thread " << QThread::currentThread();
		});
		// set resolve timer
		QTimer::singleShot(1200, [deferred2]() mutable {
			int    iNum = 666;
			double dNum = 666.666;
			qDebug() << "[INFO] DEF2::Resolved in 2 thread ********" << QThread::currentThread() << "********";
			deferred2.resolve(iNum, dNum);
		});
	};
	// post event for object with correct thread affinity
	QCoreApplication::postEvent(mp_worker, p_Evt);
}
