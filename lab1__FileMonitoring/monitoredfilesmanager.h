#ifndef MONITOREDFILESMANAGER_H
#define MONITOREDFILESMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include "filestatesnapshot.h"

class MonitoredFilesManager : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(MonitoredFilesManager)

private:
    explicit MonitoredFilesManager(QObject *parent = nullptr);
    ~MonitoredFilesManager() override;

    std::unique_ptr<FileStateSnapshot> m_previousState;
    std::unique_ptr<FileStateSnapshot> m_currentState;
    QStringList m_trackedFilePaths;

    static MonitoredFilesManager* m_instance;
    QString m_configFilePath;
    bool m_isInitialized;
    bool m_isFirstEverCheck;

    void loadTrackedFilePaths(const QString& configFilePath);
    void performStateCheckAndEmitSignals();

public:
    static MonitoredFilesManager* instance();
    bool initialize(const QString& configFilePath);
    void checkTrackedFiles();
    bool isInitializedSuccessfully() const;
    static void cleanupInstance();

signals:
    void managerFileCreated(const QString &filePath);
    void managerFileDeleted(const QString &filePath);
    void managerFileModified(const QString &filePath);
};

#endif // MONITOREDFILESMANAGER_H
