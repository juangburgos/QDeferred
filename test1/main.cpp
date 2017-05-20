#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

#include <QDeferred>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QDeferred deferred1;
	QDeferred deferred2;
	QDeferred deferred3;

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
	deferred2.fail([=](QList<QVariant> listArgs) {
		// print args
		for (int i = 0; i < listArgs.length(); i++)
		{
			qDebug() << "[DEF2] Argument " << i << " = " << listArgs.at(i) << ".";
		}
	});
	deferred2.fail([=](QList<QVariant> listArgs) {
		// print finished
		qDebug() << "[DEF2] Failed.";
	});

	// setup deferred 3
	deferred3.done([=](QList<QVariant> listArgs) {
		// print args
		for (int i = 0; i < listArgs.length(); i++)
		{
			qDebug() << "[DEF3] Argument " << i << " = " << listArgs.at(i) << ".";
		}
	});
	deferred3.done([=](QList<QVariant> listArgs) {
		// print finished
		qDebug() << "[DEF3] Resolved.";
	});

	// setup when : TODO : fails because we need to pass by referrence, 
	//                     better implement as QExplicitlySharedDataPointer for this to work
	//                     checkout QJsNode and QJsNodeData implementation
	QList<QDeferred> listDeferred;
	listDeferred.append(deferred1);
	listDeferred.append(deferred3);
	QDeferred::when(listDeferred).done([=](QList<QVariant> listArgs) {
		// print finished
		qDebug() << "[INFO] Deferred 1 and 3 Resolved.";
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
		deferred2.reject(listArgs);
	});

	// asynch resolve of deferred 3
	QTimer::singleShot(4000, [&]() {
		QList<QVariant> listArgs;
		listArgs.append("Bonjour");
		listArgs.append("Le monde");
		listArgs.append(-13);
		// resolve
		deferred3.resolve(listArgs);
	});

    return a.exec();
}
