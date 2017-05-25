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
	auto p_thread = QThread::currentThread();
	qDebug() << "[INFO] 1 thread id = " << p_thread;
	// set done
	m_deferred1.done([p_thread]() {
		qDebug() << "[INFO] DEF1::Callback defined in 1 thread " << p_thread << ", exec in thread " << QThread::currentThread();
	});
	m_deferred2.done([p_thread](int i, double d) {
		Q_UNUSED(i)
		Q_UNUSED(d)
		qDebug() << "[INFO] DEF2::Callback defined in 1 thread " << p_thread << ", exec in thread " << QThread::currentThread();
	});
	// set resolve timer
	QTimer::singleShot(1000, [&]() {
		qDebug() << "[INFO] DEF1::Resolved in 1 thread ********" << QThread::currentThread() << "********";
		m_deferred1.resolve();
	});
}

void threadWorker::doWork2()
{
	// print id
	auto p_thread = QThread::currentThread();
	qDebug() << "[INFO] 2 thread id = " << p_thread;
	// set done
	m_deferred2.done([p_thread](int i, double d) {
		Q_UNUSED(i)
		Q_UNUSED(d)
		qDebug() << "[INFO] DEF2::Callback defined in 2 thread " << p_thread << ", exec in thread " << QThread::currentThread();
	});
	m_deferred1.done([p_thread]() {
		qDebug() << "[INFO] DEF1::Callback defined in 2 thread " << p_thread << ", exec in thread " << QThread::currentThread();
	});
	// set resolve timer
	QTimer::singleShot(1200, [&]() {
		int    iNum = 666;
		double dNum = 666.666;
		qDebug() << "[INFO] DEF2::Resolved in 2 thread ********" << QThread::currentThread() << "********";
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
	m_workerThread.quit();
	m_workerThread.wait();
}

void threadController::setDeferred1(QDefer deferred1)
{
	mp_worker->setDeferred1(deferred1);
}

void threadController::setDeferred2(QDeferred<int, double> deferred2)
{
	mp_worker->setDeferred2(deferred2);
}
