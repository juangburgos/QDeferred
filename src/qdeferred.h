#ifndef QDEFERRED_H
#define QDEFERRED_H

#include <functional>

#include <QList>
#include <QVariant>

class QDeferred
{
public:
    QDeferred();

	// consumer API

	// done method
	void done(std::function<void(QList<QVariant>)> callback);
	// fail method
	void fail(std::function<void(QList<QVariant>)> callback);
	// when method
	static QDeferred when(QList<QDeferred> listDeferred);

	// provider API

	// resolve method
	void resolve(QList<QVariant> &listArgs);
	// reject method
	void reject(QList<QVariant> &listArgs);

private:
	// members
	QList<std::function<void(QList<QVariant>)>> m_doneList;
	QList<std::function<void(QList<QVariant>)>> m_failList;
	// methods
	void execute(QList<std::function<void(QList<QVariant>)>> &listCallbacks, QList<QVariant> &listArgs);

};


#endif // QDEFERRED_H