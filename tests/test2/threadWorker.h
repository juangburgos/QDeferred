#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QDeferred>

// THREADWORKER -----------------------------------------------------

class threadWorker : public QObject
{
	Q_OBJECT
public:
	void setDeferred1(QDefer deferred1);
	void setDeferred2(QDeferred<int, double> deferred2);

public slots:
	void doWork1();
	void doWork2();

private:
	QDefer                 m_deferred1;
	QDeferred<int, double> m_deferred2;
};

// THREADCONTROLLER -----------------------------------------------

class threadController : public QObject
{
	Q_OBJECT
public:
	threadController();
	~threadController();

	void setDeferred1(QDefer                 deferred1);
	void setDeferred2(QDeferred<int, double> deferred2);

signals:
	void operate1();
	void operate2();

private:
	QThread        m_workerThread;
	threadWorker * mp_worker;
};

#endif // THREAD_H
