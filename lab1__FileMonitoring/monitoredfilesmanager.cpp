#include "monitoredfilesmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

MonitoredFilesManager* MonitoredFilesManager::m_instance = nullptr;

MonitoredFilesManager::MonitoredFilesManager(QObject *parent)
    : QObject(parent), m_isInitialized(false) {
    m_previousState = new FileStateSnapshot();
    m_currentState = new FileStateSnapshot();
}

MonitoredFilesManager::~MonitoredFilesManager() {
    delete m_previousState;
    delete m_currentState;
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
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qInfo() << "MonitoredFilesManager: The configuration file could not be opened:" << configFilePath;
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
    m_isInitialized = !m_trackedFilePaths.isEmpty();
}

bool MonitoredFilesManager::initialize(const QString& configFilePath) {
    m_configFilePath = configFilePath;
    loadTrackedFilePaths(configFilePath);
    return m_isInitialized;
}

void MonitoredFilesManager::performStateCheckAndEmitSignals() {
    bool isFirstCheck = (m_previousState->count() == 0);
    for (int i = 0; i < m_trackedFilePaths.count(); ++i) {
        FileInfoData currentInfo = m_currentState->getFileInfo(i);
        if (isFirstCheck) {
            if (!currentInfo.exists) {
                emit managerFileDeleted(currentInfo.filePath);
            }
        } else {
            FileInfoData previousInfo = m_previousState->getFileInfo(i);
            if (currentInfo.exists && !previousInfo.exists) {
                emit managerFileCreated(currentInfo.filePath, currentInfo.size);
            } else if (!currentInfo.exists && previousInfo.exists) {
                emit managerFileDeleted(currentInfo.filePath);
            } else if (currentInfo.exists && previousInfo.exists) {
                if (currentInfo.size != previousInfo.size) {
                    emit managerFileModified(currentInfo.filePath, currentInfo.size);
                }
            }
        }
    }
}

void MonitoredFilesManager::checkTrackedFiles() {
    if (!m_isInitialized) return;
    m_currentState->populate(m_trackedFilePaths);
    performStateCheckAndEmitSignals();
    m_previousState->copyFrom(*m_currentState);
}
