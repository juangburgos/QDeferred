#include "threadWorker.h"

#include <QTimer>
#include <QDebug>

// THREADWORKER -----------------------------------------------------

void threadWorker::setDeferred1(QDefer deferred1)
{
	m_deferred1 = deferred1;
}

void threadWorker::setDeferred2(QDeferred<int, double> deferred2)
{
	m_deferred2 = deferred2;
}

void threadWorker::doWork1()
{
	// print id
	qDebug() << "[INFO] FIRST thread id = " << QThread::currentThreadId();
	// set done
	m_deferred1.done([]() {
		qDebug() << "[INFO] THREADWORKER::DOWORK1 callback for DEF1 executed in thread " << QThread::currentThreadId();
	});
	// set resolve timer
	QTimer::singleShot(1000, [&]() {
		m_deferred1.resolve();
	});
}

void threadWorker::doWork2()
{
	// print id
	qDebug() << "[INFO] SECOND thread id = " << QThread::currentThreadId();
	// set done
	m_deferred2.done([](int i, double d) {
		qDebug() << "[INFO] THREADWORKER::DOWORK2 callback for DEF2 executed in thread " << QThread::currentThreadId();
	});
	// set resolve timer
	QTimer::singleShot(1000, [&]() {
		int    iNum = 666;
		double dNum = 666.666;
		m_deferred2.resolve(iNum, dNum);
	});
}


// THREADCONTROLLER -----------------------------------------------

threadController::threadController()
{
	mp_worker = new threadWorker;
	mp_worker->moveToThread(&m_workerThread);
	connect(&m_workerThread, &QThread::finished         , mp_worker, &QObject::deleteLater);
	connect(this           , &threadController::operate1, mp_worker, &threadWorker::doWork1);
	connect(this           , &threadController::operate2, mp_worker, &threadWorker::doWork2);
	m_workerThread.start();
}

threadController::~threadController()
{

}

void threadController::setDeferred1(QDefer deferred1)
{
	mp_worker->setDeferred1(deferred1);
}

void threadController::setDeferred2(QDeferred<int, double> deferred2)
{
	mp_worker->setDeferred2(deferred2);
}
