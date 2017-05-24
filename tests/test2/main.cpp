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

// TODO : make thread safe!
/*
An idea is to keep an internal QMap<QThread, QObject>, one QObject per thread
then assign each std::function to its own thread (QObject).
When the deferred is either resolved or rejected, then post an special event
to each of the objects with the arguments.
Then in the QObject overwrite the event method to execute its own std::function's
that will be executed in the QObject's thread affinity due to Qt's event loop.
(hopefully)
*/

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QDefer                 deferred1;
	QDeferred<int, double> deferred2;

	qDebug() << "[INFO] MAIN thread id = " << QThread::currentThreadId();

	deferred1.done([]() {
		qDebug() << "[INFO] MAINTHD callback DEF1 exec thread " << QThread::currentThreadId();
	});
	deferred2.done([](int i, double d) {
		Q_UNUSED(i)
		Q_UNUSED(d)
		qDebug() << "[INFO] MAINTHD callback DEF2 exec thread " << QThread::currentThreadId();
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


