#include <QCoreApplication>
#include <QDebug>

#include <QDynamicEvents>
#include <QLambdaThreadWorker>

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
	// init threads
	QThread * pMainThread = QThread::currentThread();
	qDebug() << "[INFO] Main thread = " << pMainThread;
	QLambdaThreadWorker worker1;
	qDebug() << "[INFO] Worker thread 1 = " << worker1.getThreadId();

	auto callback = [](int iVal) {
		qDebug() << "[INFO] Handled callback with  " << iVal << " in thread = " << QThread::currentThread();
	};

	for (int i = 0; i < 1000000; i++)
	{
		// setup event
		QDynamicEvents<int> eventer;
		eventer.on(" number", callback);
		// resolve in different thread
		worker1.execInThread([eventer, i]() mutable {
			eventer.trigger("number ", i);
			qDebug() << "[INFO] Triggered with  " << i << " in thread = " << QThread::currentThread();
		});
		// do not block event loop
		QCoreApplication::processEvents();
	}

    return a.exec();
}

