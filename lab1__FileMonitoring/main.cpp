#include <QCoreApplication>
#include <QTextStream>
#include <QString>
#include <thread>
#include <chrono>
#include <memory>

#include <QDebug>
#include "ilogger.h"
#include "consolelogger.h"
#include "monitoredfilesmanager.h"


class SignalProcessor : public QObject {
    Q_OBJECT
public:
    explicit SignalProcessor(ILogger* logger, QObject *parent = nullptr)
        : QObject(parent), m_logger(logger) {


        if (!m_logger) {
            qWarning("SignalProcessor: Logger is nullptr!");

        }
    }

public slots:
    void onFileCreated(const QString &filePath) {
        QString message = QString("Файл '%1': СОЗДАН/ПОЯВИЛСЯ.").arg(filePath);
        if (m_logger) m_logger->log(message);
    }

    void onFileDeleted(const QString &filePath) {

        QString message = QString("Файл '%1': НЕ НАЙДЕН или УДАЛЕН.").arg(filePath);
        if (m_logger) m_logger->log(message);
    }

    void onFileModified(const QString &filePath) {
        QString message = QString("Файл '%1': ИЗМЕНЕН.").arg(filePath);
        if (m_logger) m_logger->log(message);
    }

private:
    ILogger* m_logger;
};


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "RUSSIAN");
    QCoreApplication a(argc, argv);


    QObject::connect(&a, &QCoreApplication::aboutToQuit, [](){
        MonitoredFilesManager::cleanupInstance();
        qInfo() << "MonitoredFilesManager cleanup called via aboutToQuit.";
    });

    QTextStream cli_out(stdout);




    QString configFileName = "C:/Qt/files_to_watch.txt";
    if (a.arguments().size() > 1) {
        configFileName = a.arguments().at(1);
    }
    cli_out << "The configuration file is being used: " << configFileName << Qt::endl;


        MonitoredFilesManager* manager = MonitoredFilesManager::instance();


    std::unique_ptr<ILogger> logger = std::make_unique<ConsoleLogger>();
    SignalProcessor signalHandler(logger.get(), &a);


    cli_out << "Connecting to the signals of the file manager..." << Qt::endl;
    QObject::connect(manager, &MonitoredFilesManager::managerFileCreated,
                     &signalHandler, &SignalProcessor::onFileCreated);
    QObject::connect(manager, &MonitoredFilesManager::managerFileDeleted,
                     &signalHandler, &SignalProcessor::onFileDeleted);
    QObject::connect(manager, &MonitoredFilesManager::managerFileModified,
                     &signalHandler, &SignalProcessor::onFileModified);
    cli_out << "The connection is completed." << Qt::endl;


    if (!manager->initialize(configFileName)) {
        cli_out << "Error: The MonitoredFilesManager could not be initialized with the file"
                << configFileName << "'." << Qt::endl;
        cli_out << "Check the error messages. The program will shut down." << Qt::endl;

        return 1;
    }

    cli_out << "------------------------------------" << Qt::endl;
    cli_out << "We're starting tracking. Press Ctrl+C to exit." << Qt::endl;
    cli_out << "(During the first check, there will be messages about undiscovered files, if any)" << Qt::endl;


    try {
        while (true) {
            manager->checkTrackedFiles();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (const std::exception& e) {

        if (logger) {
            logger->log(QString("CRITICAL ERROR in the main loop: %1").arg(e.what()));
        } else {
            cli_out << "CRITICAL ERROR in the main loop (the logger is unavailable): " << e.what() << Qt::endl;
        }

        return 1;
    }



    return 0;
}


#include "main.moc"
