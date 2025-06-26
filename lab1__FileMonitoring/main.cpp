#include <QCoreApplication>
#include <QTextStream>
#include <QThread>
#include <QDebug>
#include <QFileInfo>

#include "ilogger.h"
#include "consolelogger.h"
#include "monitoredfilesmanager.h"

class SignalProcessor : public QObject {
    Q_OBJECT
public:
    SignalProcessor(MonitoredFilesManager* manager, ILogger* logger, QObject *parent = nullptr)
        : QObject(parent), m_logger(logger)

    {
        QObject::connect(manager, &MonitoredFilesManager::managerFileCreated,
                         this, &SignalProcessor::onFileCreated);
        QObject::connect(manager, &MonitoredFilesManager::managerFileDeleted,
                         this, &SignalProcessor::onFileDeleted);
        QObject::connect(manager, &MonitoredFilesManager::managerFileModified,
                         this, &SignalProcessor::onFileModified);
    }

public slots:
    void onFileCreated(const QString &filePath, qint64 size) {
        QString fileName = QFileInfo(filePath).fileName();
        QString message = QString("Файл '%1': СОЗДАН/ПОЯВИЛСЯ. Размер: %2 байт.").arg(fileName).arg(size);
        if(m_logger) m_logger->log(message);
    }

    void onFileDeleted(const QString &filePath) {
        QString fileName = QFileInfo(filePath).fileName();
        QString message = QString("Файл '%1': НЕ НАЙДЕН или УДАЛЕН.").arg(fileName);
        if(m_logger) m_logger->log(message);
    }

    void onFileModified(const QString &filePath, qint64 newSize) {
        QString fileName = QFileInfo(filePath).fileName();
        QString message = QString("Файл '%1': ИЗМЕНЕН. Новый размер: %2 байт.").arg(fileName).arg(newSize);
                          if(m_logger) m_logger->log(message);
    }

private:
    ILogger* m_logger;
};

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "Russian");
    QCoreApplication a(argc, argv);
    QTextStream cli_out(stdout);

    QString configFileName = "C:/Qt/files_to_watch.txt";

    cli_out << "The configuration file is being used:: " << configFileName << Qt::endl;

    MonitoredFilesManager* manager = MonitoredFilesManager::instance();
    ILogger* logger = new ConsoleLogger();
    SignalProcessor signalHandler(manager, logger, &a);


    if (!manager->initialize(configFileName)) {
        cli_out << "Error: Failed to initialize the manager with the file '"
                << configFileName << "'." << Qt::endl;
        delete logger;
        return 1;
    }

    cli_out << "------------------------------------" << Qt::endl;
    cli_out << "Tracking started. Press Ctrl+C to exit." << Qt::endl;

    while (true) {
        manager->checkTrackedFiles();
        QThread::msleep(100);
    }

    delete logger;
    return 0;
}

#include "main.moc"
