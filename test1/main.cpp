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

void testShared(QDeferred<int, double> def) {
	qDebug() << "[INFO] Called testShared ";
	// setup deferred 3
	def.done([=](int i, double d) {
		// print args
		qDebug() << "[DEF3] testShared i = " << i << ".";
		qDebug() << "[DEF3] testShared d = " << d << ".";
	});
}

// extra API
// when method
QDeferred<> when(QList<QDeferred<>> listDeferred)
{
	qDebug() << "QDeferred<>::when called";
	// create deferred and resolve/fail it when all input deferreds have resolved/failed
	QDeferred<> retDeferred;
	// loop all input deferreds
	for (int i = 0; i < listDeferred.length(); i++)
	{
		qDebug() << "Loop " << i;
		// NOTE : Requires mutable because by default a function object should produce the same result every time it's called
		// http://stackoverflow.com/questions/5501959/why-does-c0xs-lambda-require-mutable-keyword-for-capture-by-value-by-defau
		listDeferred[i].done([listDeferred, retDeferred]() mutable {
			static int iResolveCount = 0;
			iResolveCount++;
			// test if all resolved
			if (iResolveCount == listDeferred.length())
			{
				// NOTE : resolve with args of last to resolve
				retDeferred.resolve();
			}
		});
	}
	// return
	return retDeferred;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QDeferred<QList<QVariant>>          deferred1;
	QDeferred<QList<QVariant>, QString> deferred2;
	QDeferred<int, double>              deferred3;
	QDeferred<>                         deferred4;
	QDeferred<>                         deferred5;

	// setup deferred 1
	deferred1.done([=](QList<QVariant> listArgs) {
		// print args
		for (int i = 0; i < listArgs.length(); i++)
		{
			qDebug() << "[DEF1] Argument " << i << " = " << listArgs.at(i) << ".";
		}
	});
	deferred1.done([=](QList<QVariant> listArgs) {
		// print finished
		qDebug() << "[DEF1] Resolved.";
	});

	// setup deferred 2
	deferred2.fail([=](QList<QVariant> listArgs, QString strMessage) {
		// print args
		for (int i = 0; i < listArgs.length(); i++)
		{
			qDebug() << "[DEF2] Argument " << i << " = " << listArgs.at(i) << ".";
		}
		qDebug() << "[DEF2] Message " << strMessage << ".";
	});
	deferred2.fail([=](QList<QVariant> listArgs, QString strMessage) {
		// print finished
		qDebug() << "[DEF2] Failed.";
	});

	// setup deferred 3
	deferred3.done([=](int i, double d) {
		// print args
		qDebug() << "[DEF3] Argument i = " << i << ".";
		qDebug() << "[DEF3] Argument d = " << d << ".";
	});
	deferred3.done([=](int i, double d) {
		Q_UNUSED(i)
		Q_UNUSED(d)
		// print finished
		qDebug() << "[DEF3] Resolved.";
	});

	testShared(deferred3);

	// setup deferred 4
	deferred4.done([=]() {
		// print finished
		qDebug() << "[DEF4] Resolved.";
	});

	// setup WHEN test
	QList<QDeferred<>> listDeferred;
	listDeferred.append(deferred4);
	listDeferred.append(deferred5);
	when(listDeferred).done([]() {
		// print finished
		qDebug() << "[INFO] Deferred 4 and 5 Resolved.";
	});

	// asynch resolve of deferred 1
	QTimer::singleShot(2000, [&]() {
		QList<QVariant> listArgs;
		listArgs.append("Hello");
		listArgs.append("World");
		listArgs.append(12345);
		// resolve
		deferred1.resolve(listArgs);
	});

	// asynch resolve of deferred 2
	QTimer::singleShot(3000, [&]() {
		QList<QVariant> listArgs;
		listArgs.append("Hola");
		listArgs.append("Mundo");
		listArgs.append(6789);
		// reject
		QString strMessage("Que dices?");
		deferred2.reject(listArgs, strMessage);
	});

	// asynch resolve of deferred 3
	QTimer::singleShot(4000, [&]() {
		// resolve
		int    iNum = 666;
		double dNum = 666.666;
		deferred3.resolve(iNum, dNum);
	});

	// asynch resolve of deferred 4
	QTimer::singleShot(1000, [&]() {
		// resolve
		deferred4.resolve();
	});

	// asynch resolve of deferred 5
	QTimer::singleShot(1500, [&]() {
		// resolve
		deferred5.resolve();
	});

    return a.exec();
}
