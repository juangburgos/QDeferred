#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

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
	QLambdaThreadWorker worker2;
	qDebug() << "[INFO] Worker thread 2 = " << worker2.getThreadId();

	QDynEvts eventer1;
	
	// handle main thread
	eventer1.on("change", [pMainThread]()  {
		qDebug() << "[INFO] Event \"change\" subscribed in thread " << pMainThread << ", handled in thread " << QThread::currentThread();
	});
	eventer1.on("sorted", [pMainThread]() {
		qDebug() << "[INFO] Event \"sorted\" subscribed in thread " << pMainThread << ", handled in thread " << QThread::currentThread();
	});

	// work in thread 1
	worker1.execInThread([eventer1]() mutable {
		QThread * pThread1 = QThread::currentThread();
		// handle thread 1
		eventer1.on("change", [pThread1]() {
			qDebug() << "[INFO] Event \"change\" subscribed in thread " << pThread1 << ", handled in thread " << QThread::currentThread();
		});
		eventer1.on("sorted", [pThread1]() {
			qDebug() << "[INFO] Event \"sorted\" subscribed in thread " << pThread1 << ", handled in thread " << QThread::currentThread();
		});
		// trigger 'change' thread 1
		qDebug() << "[INFO] Event \"change\" triggered in thread " << QThread::currentThread();
		eventer1.trigger("change");
	});

	// work in thread 2
	worker2.execInThread([eventer1]() mutable {
		QThread * pThread2 = QThread::currentThread();
		// trigger 'sorted' thread 1
		qDebug() << "[INFO] Event \"sorted\" triggered in thread " << QThread::currentThread();
		eventer1.trigger("sorted");
		// handle thread 2
		eventer1.on("change", [pThread2]() {
			qDebug() << "[INFO] Event \"change\" subscribed in thread " << pThread2 << ", handled in thread " << QThread::currentThread();
		});
		auto evtHandle = eventer1.on("sorted", [pThread2]() {
			qDebug() << "[INFO] Event \"sorted\" subscribed in thread " << pThread2 << ", handled in thread " << QThread::currentThread();
		});
		// cyclic triggering and off testing
		QTimer *timer = new QTimer();
		// since timer is on heap, we must delete it eventually
		QObject::connect(QThread::currentThread(), &QThread::finished, [timer]() {
			timer->stop();
			timer->deleteLater();
		});
		// set timer timeout
		QObject::connect(timer, &QTimer::timeout, [eventer1, evtHandle]() mutable {
			static int  counter       = 0;
			counter++;
			// off conditions
			if (counter == 5)
			{
				eventer1.off("change");
			} 
			else if (counter == 10)
			{
				eventer1.off(evtHandle);
			}
			else if (counter == 15)
			{
				eventer1.off();
			}
			else if (counter == 20)
			{
				QCoreApplication::quit();
			}
			// trigger 'change' thread 2
			qDebug() << "[INFO] Event \"change\" triggered in thread " << QThread::currentThread();
			eventer1.trigger("change");
			// trigger 'sorted' thread 2
			qDebug() << "[INFO] Event \"sorted\" triggered in thread " << QThread::currentThread();
			eventer1.trigger("sorted");
		});
		// start timer
		timer->start(1000);
	});

	// trigger 'change' main thread
	qDebug() << "[INFO] Event \"change\" triggered in thread " << QThread::currentThread();
	eventer1.trigger("change");

    return a.exec();
}

