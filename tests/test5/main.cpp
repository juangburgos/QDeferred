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
	qDebug() << "[INFO] Main thread = " << QThread::currentThread();

	QDynEvts eventer;
	
	eventer.on("change", []()  {
		qDebug() << "[INFO] Riverside motherfocker!";
	});

	eventer.trigger("change");
	//QLambdaThreadWorker worker;
	//qDebug() << "[INFO] Worker thread = " << worker.getThreadId();
	//// do something in different thread
	//worker.execInThread([]() mutable {
	//	qDebug() << "[INFO] Call execInThread in thread = " << QThread::currentThread();

	//});

    return a.exec();
}

