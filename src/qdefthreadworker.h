#ifndef QDEFTHREADWORKER_H
#define QDEFTHREADWORKER_H

#include <QExplicitlySharedDataPointer>
#include "qdefthreadworkerdata.h"

class QDefThreadWorker
{
public:
	// constructors
	QDefThreadWorker();
	QDefThreadWorker(const QDefThreadWorker &other);
	QDefThreadWorker &operator=(const QDefThreadWorker &rhs);
	~QDefThreadWorker();

	void execInThread(std::function<void()> threadFunc);

	QString getThreadId();

protected:
	QExplicitlySharedDataPointer<QDefThreadWorkerData> m_data;

};

#endif // QDEFTHREADWORKER_H