#ifndef AWAITWIDGET_H
#define AWAITWIDGET_H

#include <QDeferred>
#include <QLambdaThreadWorker>

static const int NumButtons = 5;

class AsyncWorker
{
public:
    AsyncWorker()
    {
        m_promises.resize(NumButtons);
    }

    ~AsyncWorker()
    {
        QDefer::await(m_thdworker.quitThread());
    }

    QDefer run(const int& promiseId)
    {
        QDefer retDef;
        m_thdworker.execInThread([this, promiseId, retDef]() mutable {
            m_promises[promiseId] = retDef;
        });
        return retDef;
    }

    void stop(const int& promiseId)
    {
        m_thdworker.execInThread([this, promiseId]() mutable {
            m_promises[promiseId].resolve();
		});
    }

private:
    QLambdaThreadWorker m_thdworker;
    QVector<QDefer> m_promises;
};

#include <QWidget>
class QPushButton;

namespace Ui {
class AwaitWidget;
}

class AwaitWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AwaitWidget(QWidget *parent = 0);
    ~AwaitWidget();

private:
    Ui::AwaitWidget *ui;
    AsyncWorker m_asyncworker;

    void startWorker(
        const int& promiseId, 
        QPushButton* startButton, 
        QPushButton* stopButton
    );
    void stopWorker(
        const int& promiseId,
        QPushButton* stopButton
    );
};

#endif // AWAITWIDGET_H
