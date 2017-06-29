#ifndef QLAMBDATHREADWORKER_H
#define QLAMBDATHREADWORKER_H

#include <QExplicitlySharedDataPointer>
#include <QDeferred>
#include "qlambdathreadworkerdata.h"

class QLambdaThreadWorker
{
public:
	// constructors
	QLambdaThreadWorker();
	QLambdaThreadWorker(const QLambdaThreadWorker &other);
	QLambdaThreadWorker &operator=(const QLambdaThreadWorker &rhs);
	~QLambdaThreadWorker();

	void    execInThread(std::function<void()> threadFunc);

	QString getThreadId();

	int     startLoopInThread(std::function<void()> threadLoopFunc, int uiMsSleep = 1000);

	bool    stopLoopInThread(const int &intLoopId);

protected:
	QExplicitlySharedDataPointer<QLambdaThreadWorkerData> m_data;

};

#endif // QLAMBDATHREADWORKER_H