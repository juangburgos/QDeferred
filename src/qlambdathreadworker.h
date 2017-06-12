#ifndef QDEFTHREADWORKER_H
#define QDEFTHREADWORKER_H

#include <QExplicitlySharedDataPointer>
#include "qlambdathreadworkerdata.h"

class QLambdaThreadWorker
{
public:
	// constructors
	QLambdaThreadWorker();
	QLambdaThreadWorker(const QLambdaThreadWorker &other);
	QLambdaThreadWorker &operator=(const QLambdaThreadWorker &rhs);
	~QLambdaThreadWorker();

	void execInThread(std::function<void()> threadFunc);

	QString getThreadId();

protected:
	QExplicitlySharedDataPointer<QLambdaThreadWorkerData> m_data;

};

#endif // QDEFTHREADWORKER_H