#ifndef QQLAMBDATHREADWORKERDATA_H
#define QQLAMBDATHREADWORKERDATA_H

#include <QThread>
#include <QObject>
#include <QEvent>
#include <QSharedData>
#include <functional>

#define QLAMBDATHREADWORKERDATA_EVENT_TYPE (QEvent::Type)(QEvent::User + 666)

 // QDEFTHREADWORKERDATAEVENT -------------------------------------------------
class QLambdaThreadWorkerDataEvent : public QEvent
{
public:
	explicit QLambdaThreadWorkerDataEvent();

	std::function<void()> m_eventFunc;

};

// QDEFTHREADWORKEROBJECTDATA -----------------------------------------------------
class QLambdaThreadWorkerObjectData : public QObject
{
	Q_OBJECT
public:
	explicit QLambdaThreadWorkerObjectData();

	bool event(QEvent* ev);

};

// QDEFTHREADWORKERDATA -----------------------------------------------------
class QLambdaThreadWorkerData : public QSharedData
{
public:
	QLambdaThreadWorkerData();
	QLambdaThreadWorkerData(const QLambdaThreadWorkerData &other);
	~QLambdaThreadWorkerData();

	void execInThread(std::function<void()> &threadFunc);

	QString getThreadId();
	
private:
	QThread                       * mp_workerThread;
	QLambdaThreadWorkerObjectData * mp_workerObj;
	QString                         m_strThreadId;
};

#endif // QQLAMBDATHREADWORKERDATA_H