#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

#include <QList>
#include <QVariant>

#include <qdeferreddata.hpp>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QDeferredData<QList<QVariant>>          deferred1;
	QDeferredData<QList<QVariant>, QString> deferred2;
	QDeferredData<int, double>              deferred3;
	QDeferredData<>                         deferred4;

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

	// setup deferred 4
	deferred4.done([=]() {
		// print finished
		qDebug() << "[DEF4] Resolved.";
	});

	////// setup when : TODO : fails because we need to pass by referrence, 
	//////                     better implement as QExplicitlySharedDataPointer for this to work
	//////                     checkout QJsNode and QJsNodeData implementation
	////QList<QDeferredData> listDeferred;
	////listDeferred.append(deferred1);
	////listDeferred.append(deferred3);
	////QDeferredData::when(listDeferred).done([=](QList<QVariant> listArgs) {
	////	// print finished
	////	qDebug() << "[INFO] Deferred 1 and 3 Resolved.";
	////});

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
	QTimer::singleShot(5000, [&]() {
		// resolve
		deferred4.resolve();
	});

    return a.exec();
}
