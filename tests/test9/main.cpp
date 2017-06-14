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

	QDynamicEvents<int> eventer1;
	
	// multiple events and trigger order test
	eventer1.on("change", [](int iVal) {
		qDebug() << "[INFO] Event \"change\" with arg = " << iVal;
	});
	eventer1.on("sorted", [](int iVal) {
		qDebug() << "[INFO] Event \"sorted\" with arg = " << iVal;
	});
	auto handle3 = eventer1.on("change sorted", [](int iVal)  {
		qDebug() << "[INFO] Event \"change & sorted\" with arg = " << iVal;
	});
	int iVal = 666;
	eventer1.trigger("change sorted", iVal);
	iVal = 333;
	eventer1.trigger("sorted change", iVal);
	eventer1.off(handle3);
	iVal = 111;
	eventer1.trigger("change sorted", iVal);
	iVal = 222;
	eventer1.trigger("sorted change", iVal);
	eventer1.off("change");
	iVal = 444;
	eventer1.trigger("change sorted", iVal);
	iVal = 555;
	eventer1.trigger("sorted change", iVal);
	eventer1.off();
	iVal = 123;
	eventer1.trigger("change sorted", iVal);
	iVal = 321;
	eventer1.trigger("sorted change", iVal);

	// tests for once method
	eventer1.once("change", [](int iVal) {
		qDebug() << "[INFO:ONCE] Event \"change\" with arg = " << iVal;
	});
	eventer1.once("sorted", [](int iVal) {
		qDebug() << "[INFO:ONCE] Event \"sorted\" with arg = " << iVal;
	});
	eventer1.once("change sorted", [](int iVal) {
		qDebug() << "[INFO:ONCE] Event \"change & sorted\" with arg = " << iVal;
	});
	iVal = 12345;
	eventer1.trigger("change sorted", iVal);
	iVal = 67890;
	eventer1.trigger("sorted change", iVal);

    return a.exec();
}

