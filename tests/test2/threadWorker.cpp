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
	qDebug() << "[INFO] 1_ST thread id = " << QThread::currentThread();
	// set done
	m_deferred1.done([]() {
		qDebug() << "[INFO] 1_WKTHD callback DEF1 exec thread " << QThread::currentThread();
	});
	m_deferred2.done([](int i, double d) {
		Q_UNUSED(i)
			Q_UNUSED(d)
			qDebug() << "[INFO] 1_WKTHD callback DEF2 exec thread " << QThread::currentThread();
	});
	// set resolve timer
	QTimer::singleShot(1000, [&]() {
		qDebug() << "[INFO] DEF1 resolved ----------------------------- ";
		m_deferred1.resolve();
	});
}

void threadWorker::doWork2()
{
	// print id
	qDebug() << "[INFO] 2_ND thread id = " << QThread::currentThread();
	// set done
	m_deferred2.done([](int i, double d) {
		Q_UNUSED(i)
		Q_UNUSED(d)
		qDebug() << "[INFO] 2_WKTHD callback DEF2 exec thread " << QThread::currentThread();
	});
	m_deferred1.done([]() {
		qDebug() << "[INFO] 2_WKTHD callback DEF1 exec thread " << QThread::currentThread();
	});
	// set resolve timer
	QTimer::singleShot(1200, [&]() {
		int    iNum = 666;
		double dNum = 666.666;
		qDebug() << "[INFO] DEF2 resolved ----------------------------- ";
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
