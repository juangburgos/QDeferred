#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>

#include <QLambdaThreadWorker>
#include <QDeferred>

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

	//for (int i = 0; i < 1000000; i++)
	//{

	QLambdaThreadWorker worker;
	QDefer stopper;
	int * p_counter = new int;
	(*p_counter) = 0;

	qDebug() << "[THREAD:STARTFROM] Id " << QThread::currentThread();

	int intLoopId = worker.startLoopInThread([p_counter, stopper]() mutable {
		(*p_counter)++;
		qDebug() << "[INFO] Iteration " << (*p_counter);
		qDebug() << "[THREAD:RUNNINGIN] Id " << QThread::currentThread();
		// stop condition
		if ((*p_counter) == 5)
		{
			stopper.reject();
		}
	}, 10);

	stopper.fail([p_counter, worker, intLoopId]() mutable {
		worker.stopLoopInThread(intLoopId);
		qDebug() << "[INFO] Stop result " << QDefer::waitBlocking(worker.stopLoopInThread(intLoopId));
		qDebug() << "[THREAD:STOPFROM] Id " << QThread::currentThread();
		delete p_counter;
	});

	//QCoreApplication::processEvents();
	//}

    return a.exec();
}

