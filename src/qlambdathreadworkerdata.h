#ifndef QQLAMBDATHREADWORKERDATA_H
#define QQLAMBDATHREADWORKERDATA_H

#include <QThread>
#include <QObject>
#include <QEvent>
#include <QSharedData>
#include <QMap>
#include <QDeferred>
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

	quint32 incrementCallbackCount();

	quint32 decrementCallbackCount();

signals:
	void finishedProcessingCallbacks();

protected:
	void timerEvent(QTimerEvent *event);

private:
	// map of 'loop' timer functions is stored here but administered in QLambdaThreadWorkerData
	// map is <timerIds, timerFuncs>
	QMap<int, std::function<void()>> m_mapFuncs;
	// make friend, so it can admin the map
	friend class QLambdaThreadWorkerData;
	// count callbacks
	quint32 m_callbacksToExec;
	// mutex to protect count
	QMutex m_mutex;
};

// QDEFTHREADWORKERDATA -----------------------------------------------------

class QLambdaThreadWorkerData : public QSharedData
{
public:
	QLambdaThreadWorkerData();
	QLambdaThreadWorkerData(const QLambdaThreadWorkerData &other);
	~QLambdaThreadWorkerData();

	bool     execInThread(const std::function<void()> &threadFunc, const Qt::EventPriority &priority = Qt::NormalEventPriority);

	QString  getThreadId();

	QThread* getThread();

	int      startLoopInThread(const std::function<void()> &threadLoopFunc, const quint32 &uiMsSleep = 1000);

	QDefer   stopLoopInThread(const int &intLoopId);

	QDefer   stopAllLoopsInThread();

	bool     moveQObjectToThread(QObject * pObject);

	QDefer   quitThread();
	
private:
	QThread                       * mp_workerThread;
	QLambdaThreadWorkerObjectData * mp_workerObj;
	QString                         m_strThreadId;
	// map of custom id, to timer id 
	// necessary because timers cannot be started/stopped on a different thread,
	// therefore we cannot get a timer id to return inmediatly
	QMap<int, int> m_mapIdtimerIds;
	int            m_intIdCounter;
	bool           m_requestedQuit;
};

#endif // QQLAMBDATHREADWORKERDATA_H