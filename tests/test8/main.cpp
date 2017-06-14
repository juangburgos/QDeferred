#include <QCoreApplication>
#include <QDebug>

#include <QLambdaThreadWorker>

#include "derived.h"

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

	Derived derived1;

	derived1.on<bool>(QString("change:boolval"), [](bool bVal) {
		qDebug() << "[INFO] Boolean value (boolval) has been changed = " << bVal;
	});
	derived1.on<int>("change:intval", [](int iVal) {
		qDebug() << "[INFO] Integer value (intval) has been changed = " << iVal;
	});
	derived1.on<double>("change:doubleval", [](double dVal) {
		qDebug() << "[INFO] Double value (doubleval) has been changed = " << dVal;
	});
	derived1.on<QString>("change:stringval", [](QString strVal) {
		qDebug() << "[INFO] String value (stringval) has been changed = " << strVal;
	});
	derived1.on<QString, QVariant>("change", [](QString strAttrName, QVariant varVal) {
		qDebug() << "[INFO] Some value (change) (" << strAttrName << ") has been changed = " << varVal;
	});

	// change once
	derived1.set_boolval(true);
	derived1.set_intval(123);
	derived1.set_doubleval(456.789);
	derived1.set_stringval("Hello");
	// change again
	derived1.set_boolval(false);
	derived1.set_intval(987);
	derived1.set_doubleval(3.1416);
	derived1.set_stringval("World");

    return a.exec();
}

