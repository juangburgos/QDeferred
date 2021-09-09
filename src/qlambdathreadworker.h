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

	bool      execInThread(const std::function<void()> &threadFunc, const Qt::EventPriority &priority = Qt::NormalEventPriority);

	QString   getThreadId();

	QThread * getThread();

	int       startLoopInThread(const std::function<void()> &threadLoopFunc, const quint32 &uiMsSleep = 1000);

	QDefer    stopLoopInThread(const int &intLoopId);

	QDefer    stopAllLoopsInThread();

	bool      moveQObjectToThread(QObject * pObject);

	QDefer    quitThread();

protected:
	QExplicitlySharedDataPointer<QLambdaThreadWorkerData> m_data;

};

#endif // QLAMBDATHREADWORKER_H