#include "qlambdathreadworker.h"

QLambdaThreadWorker::QLambdaThreadWorker() : m_data(nullptr)
{
	m_data = QExplicitlySharedDataPointer<QLambdaThreadWorkerData>(new QLambdaThreadWorkerData());
}

QLambdaThreadWorker::QLambdaThreadWorker(const QLambdaThreadWorker &other) : m_data(other.m_data)
{
	m_data.reset();
	m_data = other.m_data;
}

QLambdaThreadWorker & QLambdaThreadWorker::operator=(const QLambdaThreadWorker &rhs)
{
	if (this != &rhs) {
		m_data.reset();
		m_data.operator=(rhs.m_data);
	}
	return *this;
}

QLambdaThreadWorker::~QLambdaThreadWorker()
{
	m_data.reset();
}

void QLambdaThreadWorker::execInThread(const std::function<void()> &threadFunc, const Qt::EventPriority &priority/* = Qt::NormalEventPriority*/)
{
	m_data->execInThread(threadFunc, priority);
}

QString QLambdaThreadWorker::getThreadId()
{
	return m_data->getThreadId();
}

QThread * QLambdaThreadWorker::getThread()
{
	return m_data->getThread();
}

int QLambdaThreadWorker::startLoopInThread(const std::function<void()> &threadLoopFunc, const quint32 &uiMsSleep /*= 1000*/)
{
	return m_data->startLoopInThread(threadLoopFunc, uiMsSleep);
}

void QLambdaThreadWorker::stopLoopInThread(const int &intLoopId)
{
	m_data->stopLoopInThread(intLoopId);
}

void QLambdaThreadWorker::moveQObjectToThread(QObject * pObject)
{
	m_data->moveQObjectToThread(pObject);
}
