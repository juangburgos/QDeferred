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

void testShared(QDeferred<int, double> def) 
{
	qDebug() << "[INFO] Called testShared ";
	// setup deferred 3
	def.doneVsDbg([=](int i, double d) {
		// print args
		qDebug() << "[DEF3] testShared i = " << i << ".";
		qDebug() << "[DEF3] testShared d = " << d << ".";
	});
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QDeferred<QList<QVariant>>          deferred1;
	QDeferred<QList<QVariant>, QString> deferred2;
	QDeferred<int, double>              deferred3;
	QDefer deferred4;
	QDefer deferred5;

	QDefer::when(deferred1, deferred4, deferred5)
	  .doneVsDbg([]() {
		qDebug() << "[INFO] First when has done.";
	}).failVsDbg([]() {
		qDebug() << "[INFO] First when has failed.";
	}).thenVsDbg([]() {
		qDebug() << "[INFO] First when has thened.";
	});

	QDefer::when(deferred2, deferred3, deferred4)
	  .doneVsDbg([]() {
		qDebug() << "[INFO] Second when has done.";
	}).failVsDbg([]() {
		qDebug() << "[INFO] Second when has failed.";
	}).thenVsDbg([]() {
		qDebug() << "[INFO] Second when has thenned.";
	});

	qDebug() << "[INFO] deferred1.state() = " << deferred1.state();
	qDebug() << "[INFO] deferred2.state() = " << deferred2.state();
	qDebug() << "[INFO] deferred3.state() = " << deferred3.state();
	qDebug() << "[INFO] deferred4.state() = " << deferred4.state();
	qDebug() << "[INFO] deferred5.state() = " << deferred5.state();

	// setup deferred 1
	deferred1.doneVsDbg([=](QList<QVariant> listArgs) {
		// print args
		for (int i = 0; i < listArgs.length(); i++)
		{
			qDebug() << "[DEF1] Argument " << i << " = " << listArgs.at(i) << ".";
		}
	}).doneVsDbg([=](QList<QVariant> listArgs) {
		// print finished
		qDebug() << "[DEF1] Resolved.";
	});

	// setup deferred 2
	deferred2.failVsDbg([=](QList<QVariant> listArgs, QString strMessage) {
		// print args
		for (int i = 0; i < listArgs.length(); i++)
		{
			qDebug() << "[DEF2] Argument " << i << " = " << listArgs.at(i) << ".";
		}
		qDebug() << "[DEF2] Message " << strMessage << ".";
	}).failVsDbg([=](QList<QVariant> listArgs, QString strMessage) {
		// print finished
		qDebug() << "[DEF2] Failed.";
	});

	// setup deferred 3
	deferred3.doneVsDbg([=](int i, double d) {
		// print args
		qDebug() << "[DEF3] Argument i = " << i << ".";
		qDebug() << "[DEF3] Argument d = " << d << ".";
	}).doneVsDbg([=](int i, double d) {
		Q_UNUSED(i)
		Q_UNUSED(d)
		// print finished
		qDebug() << "[DEF3] Resolved.";
	});

	testShared(deferred3);

	// setup deferred 4
	deferred4.doneVsDbg([=]() {
		// print finished
		qDebug() << "[DEF4] Resolved.";
	});

	// setup deferred 5
	deferred5.thenVsDbg([=]() {
		// print finished
		qDebug() << "[DEF5] Thenned.";
	});



	// asynch resolve of deferred 1
	QTimer::singleShot(2000, [&]() {
		QList<QVariant> listArgs;
		listArgs.append("Hello");
		listArgs.append("World");
		listArgs.append(12345);
		// resolve
		deferred1.resolveVsDbg(listArgs);
		qDebug() << "[INFO] deferred1.state() = " << deferred1.state();
	});

	// asynch resolve of deferred 2
	QTimer::singleShot(3000, [&]() {
		QList<QVariant> listArgs;
		listArgs.append("Hola");
		listArgs.append("Mundo");
		listArgs.append(6789);
		// reject
		QString strMessage("Que dices?");
		deferred2.rejectVsDbg(listArgs, strMessage);
		qDebug() << "[INFO] deferred2.state() = " << deferred2.state();
	});

	// asynch resolve of deferred 3
	QTimer::singleShot(4000, [&]() {
		// resolve
		int    iNum = 666;
		double dNum = 666.666;
		deferred3.resolveVsDbg(iNum, dNum);
		qDebug() << "[INFO] deferred3.state() = " << deferred3.state();
		// call done again
		deferred3.doneVsDbg([=](int i, double d) {
			// print args
			qDebug() << "[DEF3] After done i = " << i << ".";
			qDebug() << "[DEF3] After done d = " << d << ".";
		});
	});

	// asynch resolve of deferred 4
	QTimer::singleShot(1000, [&]() {
		// resolve
		deferred4.resolveVsDbg();
		qDebug() << "[INFO] deferred4.state() = " << deferred4.state();
	});

	// asynch resolve of deferred 5
	QTimer::singleShot(1500, [&]() {
		// resolve
		deferred5.resolveVsDbg();
		qDebug() << "[INFO] deferred5.state() = " << deferred5.state();
		deferred5.doneVsDbg([=]() {
			// print finished
			qDebug() << "[DEF5] After done.";
		});
	});

    return a.exec();
}

