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

	qDebug() << "[INFO] Main thread = " << QThread::currentThread();

	ThreadController controller;
	// setup
	controller.doProgressWork(100, 30).progress([](int num) {
		qDebug() << "[INFO] Progress callback with " << num << " in thread = " << QThread::currentThread();
	}).done([](int num) {
		qDebug() << "[INFO] Done callback with  " << num << " in thread = " << QThread::currentThread();
	}).fail([](int num) {
		qDebug() << "[INFO] Fail callback with  " << num << " in thread = " << QThread::currentThread();
	});
	
    return a.exec();
}

