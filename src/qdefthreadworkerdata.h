#ifndef QDEFTHREADWORKERDATA_H
#define QDEFTHREADWORKERDATA_H

#include <QThread>
#include <QObject>
#include <QEvent>
#include <QSharedData>
#include <functional>

#define QDEFTHREADWORKERDATA_EVENT_TYPE (QEvent::Type)(QEvent::User + 666)

 // QDEFTHREADWORKERDATAEVENT -------------------------------------------------
class QDefThreadWorkerDataEvent : public QEvent
{
public:
	explicit QDefThreadWorkerDataEvent();

	std::function<void()> m_eventFunc;

};

// QDEFTHREADWORKEROBJECTDATA -----------------------------------------------------
class QDefThreadWorkerObjectData : public QObject
{
	Q_OBJECT
public:
	explicit QDefThreadWorkerObjectData();

	bool event(QEvent* ev);

};

// QDEFTHREADWORKERDATA -----------------------------------------------------
class QDefThreadWorkerData : public QSharedData
{
public:
	QDefThreadWorkerData();
	QDefThreadWorkerData(const QDefThreadWorkerData &other);
	~QDefThreadWorkerData();

	void execInThread(std::function<void()> &threadFunc);

	QString getThreadId();
	
private:
	QThread                    * mp_workerThread;
	QDefThreadWorkerObjectData * mp_workerObj;
	QString                      m_strThreadId;
};

#endif // QDEFTHREADWORKERDATA_H