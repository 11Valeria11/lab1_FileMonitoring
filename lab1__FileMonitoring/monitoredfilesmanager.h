#ifndef MONITOREDFILESMANAGER_H
#define MONITOREDFILESMANAGER_H

#include <QObject>
#include <QStringList>
#include "filestatesnapshot.h"

class MonitoredFilesManager : public QObject {
    Q_OBJECT

private:
    MonitoredFilesManager(QObject *parent = nullptr);
    ~MonitoredFilesManager();

    FileStateSnapshot* m_previousState;
    FileStateSnapshot* m_currentState;
    QStringList m_trackedFilePaths;
    static MonitoredFilesManager* m_instance;
    QString m_configFilePath;
    bool m_isInitialized;

    void loadTrackedFilePaths(const QString& configFilePath);
    void performStateCheckAndEmitSignals();

public:
    static MonitoredFilesManager* instance();
    bool initialize(const QString& configFilePath);
    void checkTrackedFiles();

signals:
    void managerFileCreated(const QString &filePath, qint64 size);
    void managerFileDeleted(const QString &filePath);
    void managerFileModified(const QString &filePath, qint64 newSize);
};

#endif // MONITOREDFILESMANAGER_H
