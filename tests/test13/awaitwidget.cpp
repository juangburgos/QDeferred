#include "awaitwidget.h"
#include "ui_awaitwidget.h"

AwaitWidget::AwaitWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AwaitWidget)
{
    ui->setupUi(this);
    // setup buttons click event handlers
    connect(ui->pushButtonAwait1, &QPushButton::clicked, this, [this]() {
        this->startWorker(1, ui->pushButtonAwait1, ui->pushButtonResolve1);
    });
    connect(ui->pushButtonAwait2, &QPushButton::clicked, this, [this]() {
        this->startWorker(2, ui->pushButtonAwait2, ui->pushButtonResolve2);
    });
    connect(ui->pushButtonAwait3, &QPushButton::clicked, this, [this]() {
        this->startWorker(3, ui->pushButtonAwait3, ui->pushButtonResolve3);
    });
    connect(ui->pushButtonAwait4, &QPushButton::clicked, this, [this]() {
        this->startWorker(4, ui->pushButtonAwait4, ui->pushButtonResolve4);
    });
    connect(ui->pushButtonAwait5, &QPushButton::clicked, this, [this]() {
        this->startWorker(5, ui->pushButtonAwait5, ui->pushButtonResolve5);
    });
    connect(ui->pushButtonResolve1, &QPushButton::clicked, this, [this]() {
        this->stopWorker(1, ui->pushButtonResolve1);
    });
    connect(ui->pushButtonResolve2, &QPushButton::clicked, this, [this]() {
        this->stopWorker(2, ui->pushButtonResolve2);
    });
    connect(ui->pushButtonResolve3, &QPushButton::clicked, this, [this]() {
        this->stopWorker(3, ui->pushButtonResolve3);
    });
    connect(ui->pushButtonResolve4, &QPushButton::clicked, this, [this]() {
        this->stopWorker(4, ui->pushButtonResolve4);
    });
    connect(ui->pushButtonResolve5, &QPushButton::clicked, this, [this]() {
        this->stopWorker(5, ui->pushButtonResolve5);
    });
}

AwaitWidget::~AwaitWidget()
{
    delete ui;
}

void AwaitWidget::startWorker(
    const int& promiseId, 
    QPushButton* startButton, 
    QPushButton* stopButton
)
{
    // change state
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    // tun in thread, wait here until resolved
    QDefer::await(m_asyncworker.run(promiseId - 1));
    // resume, reset state
    startButton->setEnabled(true);
    stopButton->setEnabled(false);
}

void AwaitWidget::stopWorker(
    const int& promiseId,
    QPushButton* stopButton
)
{
    // do not allow to resolve twice
    stopButton->setEnabled(false);
    // resolve in thread
    m_asyncworker.stop(promiseId - 1);
}
