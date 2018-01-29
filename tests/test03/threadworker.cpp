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
	QObject::connect(&m_workerThread, &QThread::finished, mp_worker, &QObject::deleteLater);
	m_workerThread.start();
}

ThreadController::~ThreadController()
{
	m_workerThread.quit();
	m_workerThread.wait();
}

QDeferred<int> ThreadController::doProgressWork(int delay, int max)
{
	QDeferred<int> retDeferred;
	// exec in thread
	ThreadWorkerEvent * p_Evt = new ThreadWorkerEvent;
	p_Evt->m_eventFunc = [retDeferred, delay, max]() mutable {
		qDebug() << "[INFO] Exec thread = " << QThread::currentThread();
		// NOTE : the timer must exists in the heap
		//        if exists on the stack will be deleted as soon as out of scope
		//        and timeout will never be triggered
		QTimer *timer = new QTimer();
		// since timer is on heap, we must delete it eventually
		QObject::connect(QThread::currentThread(), &QThread::finished, [timer]() {
			timer->stop();
			timer->deleteLater();
			qDebug() << "[INFO] Finished timer... Hasta la vista baby!";
			QThread::msleep(6000);
		});
		// set timer timeout
		QObject::connect(timer, &QTimer::timeout, [retDeferred, max]() mutable {
			static int counter = 0;
			counter++;
			if (counter < max || (counter > max && counter <= 1.1*max))
			{
				qDebug() << "[INFO] Deferred notify with " << counter << " in thread = " << QThread::currentThread();
				if (retDeferred.state() == PENDING)
				{
					retDeferred.notify(counter);
				}
			} 
			else if (counter == max)
			{
				qDebug() << "[INFO] Deferred reject with " << counter << " in thread = " << QThread::currentThread();
				retDeferred.reject(counter);
			}
			else
			{
				QCoreApplication::quit();
			}
		});
		// start timer
		timer->start(delay);
	};
	// post event for object with correct thread affinity
	QCoreApplication::postEvent(mp_worker, p_Evt);
	// return unresolved deferred
	return retDeferred;
}