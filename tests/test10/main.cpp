#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

#include <QEventer>

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

		QEventer eventer1;

		// multiple events and trigger order test
		eventer1.on<int>("change", [](int iVal) {
			qDebug() << "[INFO] Event \"change\" with arg = " << iVal;
		});
		eventer1.on<double>("sorted", [](double fVal) {
			qDebug() << "[INFO] Event \"sorted\" with arg = " << fVal;
		});
		auto handle3 = eventer1.on<QString>("change sorted", [](QString strVal) {
			qDebug() << "[INFO] Event \"change & sorted\" with arg = " << strVal;
		});
		eventer1.on<QString, QVariant>("custom", [](QString strVal, QVariant varVal) {
			qDebug() << "[INFO] Event \"" << strVal << "\" with arg = " << varVal;
		});

		int     iVal = 666;
		double  fVal = 3.1416;
		QString strVal = "hello";
		eventer1.trigger<int>("change sorted", iVal);
		eventer1.trigger<double>("change sorted", fVal);
		eventer1.trigger<QString>("change sorted", strVal);
		eventer1.trigger<QString, QVariant>("custom", "CUSTOM", "Hello World!");

		eventer1.off("sorted");

		iVal = 666;
		fVal = 3.1416;
		strVal = "WORLD";
		eventer1.trigger<int>("change sorted", iVal);
		eventer1.trigger<double>("change sorted", fVal);
		eventer1.trigger<QString>("change sorted", strVal);

	//	// do not block event loop
	//	QCoreApplication::processEvents();
	//}

    return a.exec();
}

