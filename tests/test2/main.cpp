#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

#include <QList>
#include <QVariant>

#include "threadWorker.h"

/*
LAMBDA CAPTURE
http://www.cprogramming.com/c++11/c++11-lambda-closures.html
[]	        Capture nothing (or, a scorched earth strategy?)
[&]	        Capture any referenced variable by reference
[=]	        Capture any referenced variable by making a copy
[=, &foo]	Capture any referenced variable by making a copy, but capture variable foo by reference
[bar]	    Capture bar by making a copy; don't copy anything else
[this]	    Capture the this pointer of the enclosing class
*/

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QDefer                 deferred1;
	QDeferred<int, double> deferred2;

	auto p_thread = QThread::currentThread();
	qDebug() << "[INFO] 0 thread id = " << p_thread;

	deferred1.done([p_thread]() {
		qDebug() << "[INFO] DEF1::Callback defined in 0 thread " << p_thread << ", exec in thread " << QThread::currentThread();
	});
	deferred2.done([p_thread](int i, double d) {
		Q_UNUSED(i)
		Q_UNUSED(d)
		qDebug() << "[INFO] DEF2::Callback defined in 0 thread " << p_thread << ", exec in thread " << QThread::currentThread();
	});
	QDefer::when(deferred1, deferred2).done([p_thread]() {
		qDebug() << "[INFO] WHEN::Callback defined in 0 thread " << p_thread << ", exec in thread " << QThread::currentThread();
	});
	
	threadController controller1;
	threadController controller2;

	controller1.setDeferred1(deferred1);
	controller1.setDeferred2(deferred2);
	controller2.setDeferred2(deferred2);
	controller2.setDeferred1(deferred1);
	
	emit controller1.operate1();
	emit controller2.operate2();
	
    return a.exec();
}

