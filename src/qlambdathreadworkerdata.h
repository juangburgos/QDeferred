#ifndef QQLAMBDATHREADWORKERDATA_H
#define QQLAMBDATHREADWORKERDATA_H

#include <QThread>
#include <QObject>
#include <QEvent>
#include <QSharedData>
#include <QMap>
////#include <QMutex>
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

protected:
	void timerEvent(QTimerEvent *event);

private:
	// map of 'loop' timer functions is stored here but administered in QLambdaThreadWorkerData
	// map is <timerIds, timerFuncs>
	QMap<int, std::function<void()>> m_mapFuncs;
	// make friend, so it can admin the map
	friend class QLambdaThreadWorkerData;
};

// QDEFTHREADWORKERDATA -----------------------------------------------------
class QLambdaThreadWorkerData : public QSharedData
{
public:
	QLambdaThreadWorkerData();
	QLambdaThreadWorkerData(const QLambdaThreadWorkerData &other);
	~QLambdaThreadWorkerData();

	void    execInThread(std::function<void()> &threadFunc);

	QString getThreadId();

	int     startLoopInThread(std::function<void()> &threadLoopFunc, int uiMsSleep = 1000);

	bool    stopLoopInThread(const int &intLoopId);
	
private:
	QThread                       * mp_workerThread;
	QLambdaThreadWorkerObjectData * mp_workerObj;
	QString                         m_strThreadId;
	// map of custom id, to timer id 
	// necessary because timers cannot be started/stopped on a different thread,
	// therefore we cannot get a timer id to return inmediatly
	QMap<int, int> m_mapIdtimerIds;
	int            m_intIdCounter;
};

#endif // QQLAMBDATHREADWORKERDATA_H