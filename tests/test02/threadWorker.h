#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QDeferred>

#define QWORKERPROXY_EVENT_TYPE (QEvent::Type)(QEvent::User + 666)

// THREADWORKEREVENT -------------------------------------------------

class ThreadWorkerEvent : public QEvent
{
public:
	explicit ThreadWorkerEvent();

	std::function<void()> m_eventFunc;

};

// THREADWORKER -----------------------------------------------------

class ThreadWorker : public QObject
{
	Q_OBJECT
public:
	explicit ThreadWorker();

	bool event(QEvent* ev);

};

// THREADCONTROLLER -----------------------------------------------

class ThreadController
{
public:
	ThreadController();
	~ThreadController();

	void doWorkOnFirstDeferred(QDefer deferred1, QDeferred<int, double> deferred2);
	void doWorkOnSecondDeferred(QDefer deferred1, QDeferred<int, double> deferred2);

	QDeferred<int> doProgressWork(int delay);

private:
	QThread        m_workerThread;
	ThreadWorker * mp_worker;
};

#endif // THREAD_H
