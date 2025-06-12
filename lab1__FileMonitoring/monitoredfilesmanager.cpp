#include "monitoredfilesmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

MonitoredFilesManager* MonitoredFilesManager::m_instance = nullptr;

MonitoredFilesManager::MonitoredFilesManager(QObject *parent)
    : QObject(parent), m_isInitialized(false), m_isFirstEverCheck(true) {
    m_previousState = std::make_unique<FileStateSnapshot>();
    m_currentState = std::make_unique<FileStateSnapshot>();
    qInfo() << "MonitoredFilesManager: Singleton created.";
}

MonitoredFilesManager::~MonitoredFilesManager() {
    qInfo() << "MonitoredFilesManager: Singleton destroyed.";
}

MonitoredFilesManager* MonitoredFilesManager::instance() {
    if (!m_instance) {
        m_instance = new MonitoredFilesManager();
    }
    return m_instance;
}

void MonitoredFilesManager::loadTrackedFilePaths(const QString& configFilePath) {
    m_trackedFilePaths.clear();
    QFile file(configFilePath);

    if (!file.exists()) {
        qWarning() << "MonitoredFilesManager: Файл конфигурации не найден:" << configFilePath;
        m_isInitialized = false;
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "MonitoredFilesManager: Не удалось открыть файл конфигурации:" << configFilePath;
        m_isInitialized = false;
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            m_trackedFilePaths.append(line);
        }
    }
    file.close();

    if (m_trackedFilePaths.isEmpty() && file.exists()) {
        qWarning() << "MonitoredFilesManager: Файл конфигурации" << configFilePath << "пуст или не содержит валидных путей.";
        m_isInitialized = false;
    } else if (!m_trackedFilePaths.isEmpty()) {
        m_isInitialized = true;
    }
}

bool MonitoredFilesManager::initialize(const QString& configFilePath) {
    // Проверяем, если уже инициализирован с тем же конфигом и первая проверка прошла
    if (m_isInitialized && configFilePath == m_configFilePath && !m_isFirstEverCheck) {
        qInfo() << "MonitoredFilesManager: Already initialized and first check done for" << configFilePath;
        return true;
    }

    m_configFilePath = configFilePath;
    m_isFirstEverCheck = true;
    m_previousState->clear();
    m_currentState->clear();

    loadTrackedFilePaths(configFilePath);

    if (!m_isInitialized || m_trackedFilePaths.isEmpty()) {
        qWarning() << "MonitoredFilesManager: Initialization failed or no files to track from" << configFilePath;
        return false;
    }

    qInfo() << "MonitoredFilesManager: Initialized successfully with" << m_trackedFilePaths.size() << "paths from" << configFilePath;
    return true;
}

void MonitoredFilesManager::performStateCheckAndEmitSignals() {
    // Убедимся, что currentState имеет правильный размер
    if (m_currentState->count() != m_trackedFilePaths.count()){
        qWarning() << "MonitoredFilesManager: currentState size mismatch. Current:"
                   << m_currentState->count() << "Expected:" << m_trackedFilePaths.count();
        return;
    }

    if (m_isFirstEverCheck) {
        for (int i = 0; i < m_trackedFilePaths.count(); ++i) {
            FileInfoData currentInfo = m_currentState->getFileInfo(i);
            if (!currentInfo.exists) {
                // При первой проверке, если файла нет, генерируем сигнал
                emit managerFileDeleted(currentInfo.filePath);
            }
            // Для существующих файлов при первой проверке ничего не выводим
        }
        m_isFirstEverCheck = false;
    } else {
        // Логика для последующих (нормальных) проверок

        if (m_previousState->count() != m_trackedFilePaths.count()){
            qWarning() << "MonitoredFilesManager: previousState size mismatch in normal check. Previous:"
                       << m_previousState->count() << "Expected:" << m_trackedFilePaths.count();

            return;
        }

        for (int i = 0; i < m_trackedFilePaths.count(); ++i) {
            FileInfoData currentInfo = m_currentState->getFileInfo(i);
            FileInfoData previousInfo = m_previousState->getFileInfo(i); // Берем по тому же индексу


            if (previousInfo.filePath != currentInfo.filePath) {
                qWarning() << "MonitoredFilesManager: File path mismatch at index" << i
                           << "Previous:" << previousInfo.filePath << "Current:" << currentInfo.filePath;
                previousInfo.filePath = currentInfo.filePath; // Корректируем для логики ниже
            }

            // Логика сравнения существования и размера
            if (currentInfo.exists != previousInfo.exists) {
                if (currentInfo.exists) { // Файл появился
                    emit managerFileCreated(currentInfo.filePath);
                } else { // Файл исчез (т.к. previousInfo.exists было true)
                    emit managerFileDeleted(currentInfo.filePath);
                }
            } else if (currentInfo.exists) { // Состояние существования не изменилось, и файл существует
                if (currentInfo.size != previousInfo.size) {
                    emit managerFileModified(currentInfo.filePath);
                }
            }

        }
    }
}

void MonitoredFilesManager::checkTrackedFiles() {
    if (!m_isInitialized || m_trackedFilePaths.isEmpty()) {
        return;
    }

    // 1. Обновляем m_currentState актуальными данными о файлах
    m_currentState->populate(m_trackedFilePaths);

    // 2. Выполняем проверку состояний и генерируем сигналы
    performStateCheckAndEmitSignals();

    // 3. Обновляем m_previousState для следующей итерации:
    //    оно должно стать таким же, как m_currentState в этой итерации.
    m_previousState->copyFrom(*m_currentState);
}

bool MonitoredFilesManager::isInitializedSuccessfully() const {
    return m_isInitialized && !m_trackedFilePaths.isEmpty();
}

void MonitoredFilesManager::cleanupInstance() {
    delete m_instance;
    m_instance = nullptr;
}
