#include <QCoreApplication>
#include <QDebug>

#include <QDeferred>
#include <QDefThreadWorker>

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

	QDefThreadWorker worker;
	qDebug() << "[INFO] Worker thread = " << worker.getThreadId();
	for (int i = 0; i < 1000000; i++)
	{
		// setup deferred
		QDeferred<int> retDeferred;
		retDeferred.doneVsDbg(callback);
		// resolve in different thread
		worker.execInThread([retDeferred, i]() mutable {
			qDebug() << "[INFO] Resolve with  " << i << " in thread = " << QThread::currentThread();
			retDeferred.resolveVsDbg(i);
		});
		// do not block event loop
		QCoreApplication::processEvents();
	}

    return a.exec();
}

