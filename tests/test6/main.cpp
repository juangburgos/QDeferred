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
	QLambdaThreadWorker * p_worker2 = new QLambdaThreadWorker;
	qDebug() << "[INFO] Worker thread 2 = " << p_worker2->getThreadId();

	QDynamicEvents<int>     eventer1;
	QDynamicEvents<QString> eventer2;
	
	// handle main thread
	eventer1.on("change sorted", [pMainThread](int iVal)  {
		qDebug() << "[INFO] Event \"change sorted\" subscribed in thread " << pMainThread << ", handled in thread " << QThread::currentThread() << " with arg = " << iVal;
	});
	eventer2.on("cambio reorden", [pMainThread](QString strVal) {
		qDebug() << "[INFO] Event \"cambio reorden\" subscribed in thread " << pMainThread << ", handled in thread " << QThread::currentThread() << " with arg = " << strVal;
	});

	// work in thread 1
	worker1.execInThread([&eventer1, &eventer2, p_worker2]() mutable {
		QThread * pThread1 = QThread::currentThread();
		// handle thread 1
		eventer1.on("change", [pThread1](int iVal) {
			qDebug() << "[INFO] Event \"change\" subscribed in thread " << pThread1 << ", handled in thread " << QThread::currentThread() << " with arg = " << iVal;
		});
		eventer2.on("cambio", [pThread1](QString strVal) {
			qDebug() << "[INFO] Event \"cambio\" subscribed in thread " << pThread1 << ", handled in thread " << QThread::currentThread() << " with arg = " << strVal;
		});
		// cyclic triggering and off testing
		QTimer *timer = new QTimer();
		// since timer is on heap, we must delete it eventually
		QObject::connect(QThread::currentThread(), &QThread::finished, [timer]() {
			timer->stop();
			timer->deleteLater();
		});
		// set timer timeout
		QObject::connect(timer, &QTimer::timeout, [&eventer1, &eventer2, p_worker2]() mutable {
			static int  counter = 0;
			counter++;
			// trigger 'change' and 'reorden' thread 1
			int iVal = 111;
			//qDebug() << "[INFO] Event \"change\" triggered in thread " << QThread::currentThread() << " with arg = " << iVal;
			eventer1.trigger("change", iVal);
			QString strVal = "Hello World";
			//qDebug() << "[INFO] Event \"reorden\" triggered in thread " << QThread::currentThread() << " with arg = " << strVal;
			eventer2.trigger("reorden", strVal);
			if (counter == 5)
			{
				// delete worker2
				delete p_worker2;
			}
			else if (counter == 10)
			{
				eventer1.off("change");
			}
			else if (counter == 15)
			{
				eventer2.off("reorden");
			}
			else if (counter == 20)
			{
				QCoreApplication::quit();
			}
		});
		// start timer
		timer->start(1000);
	});

	// work in thread 2
	p_worker2->execInThread([&eventer1, &eventer2]() mutable {
		QThread * pThread2 = QThread::currentThread();
		// handle main thread
		eventer1.on("sorted", [pThread2](int iVal) {
			qDebug() << "[INFO] Event \"sorted\" subscribed in thread " << pThread2 << ", handled in thread " << QThread::currentThread() << " with arg = " << iVal;
		});
		eventer2.on("reorden", [pThread2](QString strVal) {
			qDebug() << "[INFO] Event \"reorden\" subscribed in thread " << pThread2 << ", handled in thread " << QThread::currentThread() << " with arg = " << strVal;
		});
		// cyclic triggering and off testing
		QTimer *timer = new QTimer();
		// since timer is on heap, we must delete it eventually
		QObject::connect(QThread::currentThread(), &QThread::finished, [timer]() {
			timer->stop();
			timer->deleteLater();
		});
		// set timer timeout
		QObject::connect(timer, &QTimer::timeout, [&eventer1, &eventer2]() mutable {
			static int  counter       = 0;
			counter++;
			// trigger 'change' and 'reorden' thread 1
			int iVal = 222;
			//qDebug() << "[INFO] Event \"sorted\" triggered in thread " << QThread::currentThread() << " with arg = " << iVal;
			eventer1.trigger("sorted", iVal);
			QString strVal = "Hola Mundo";
			//qDebug() << "[INFO] Event \"cambio\" triggered in thread " << QThread::currentThread() << " with arg = " << strVal;
			eventer2.trigger("cambio", strVal);
		});
		// start timer
		timer->start(1000);
	});

	// trigger 'change' and 'cambio' main thread
	int iVal = 666;
	//qDebug() << "[INFO] Event \"change\" triggered in thread " << QThread::currentThread() << " with arg = " << iVal;
	eventer1.trigger("change", iVal);
	QString strVal = "Il diavolo";
	//qDebug() << "[INFO] Event \"cambio\" triggered in thread " << QThread::currentThread() << " with arg = " << strVal;
	eventer2.trigger("cambio", strVal);

    return a.exec();
}

