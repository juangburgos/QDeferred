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

	auto callback = [](int num) {
		qDebug() << "[INFO] Done callback with  " << num << " in thread = " << QThread::currentThread();
	};

	ThreadController controller;
	for (int i = 0; i < 1000000; i++)
	{
		// setup
		controller.doSomeWork(i).done(callback);	
		// do not block event loop
		QCoreApplication::processEvents();
		// sleep a little
		// [BUG] MEMORY LEAK requires no lambdas in QDeferredData<Types...>::getCallbaksForThread
		//       AND at least a 1ms sleep (other thread's eventloop needs time to delete its events)
		QThread::msleep(1);
	}

    return a.exec();
}

