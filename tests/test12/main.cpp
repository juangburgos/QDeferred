#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

#include <QList>
#include <QVariant>

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

	// memory bug
	QDefer deferred1;
	
	// resolve first
	deferred1.resolve();
	// then add a bunch of done callbacks
	for (int i = 0; i < 100000; i++)
	{
		// capture by copy some large data
		deferred1.done([i]() {
			qInfo() << "[INFO] Done " << i;
		});
		// sleep a little
		QThread::msleep(10);
	}
	//
    return a.exec();
}


