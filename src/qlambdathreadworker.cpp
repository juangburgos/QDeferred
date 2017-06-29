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

void QLambdaThreadWorker::execInThread(std::function<void()> threadFunc)
{
	m_data->execInThread(threadFunc);
}

QString QLambdaThreadWorker::getThreadId()
{
	return m_data->getThreadId();
}

int QLambdaThreadWorker::startLoopInThread(std::function<void()> threadLoopFunc, int uiMsSleep /*= 1000*/)
{
	return m_data->startLoopInThread(threadLoopFunc, uiMsSleep);
}

bool QLambdaThreadWorker::stopLoopInThread(const int &intLoopId)
{
	return m_data->stopLoopInThread(intLoopId);
}

void QLambdaThreadWorker::moveQObjectToThread(QObject * pObject)
{
	m_data->moveQObjectToThread(pObject);
}
