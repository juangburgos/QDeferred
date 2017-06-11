#include "qdefthreadworker.h"

QDefThreadWorker::QDefThreadWorker() : m_data(nullptr)
{
	m_data = QExplicitlySharedDataPointer<QDefThreadWorkerData>(new QDefThreadWorkerData());
}

QDefThreadWorker::QDefThreadWorker(const QDefThreadWorker &other) : m_data(other.m_data)
{
	m_data.reset();
	m_data = other.m_data;
}

QDefThreadWorker & QDefThreadWorker::operator=(const QDefThreadWorker &rhs)
{
	if (this != &rhs) {
		m_data.reset();
		m_data.operator=(rhs.m_data);
	}
	return *this;
}

QDefThreadWorker::~QDefThreadWorker()
{
	m_data.reset();
}

void QDefThreadWorker::execInThread(std::function<void()> threadFunc)
{
	m_data->execInThread(threadFunc);
}

QString QDefThreadWorker::getThreadId()
{
	return m_data->getThreadId();
}
