#ifndef FILESTATESNAPSHOT_H
#define FILESTATESNAPSHOT_H

#include <QList>
#include <QStringList>

struct FileInfoData {
    QString filePath;
    bool exists;
    qint64 size;

    FileInfoData(const QString& path = QString(), bool ex = false, qint64 sz = -1)
        : filePath(path), exists(ex), size(sz) {}
};

class FileStateSnapshot {
public:
    FileStateSnapshot();
    void populate(const QStringList& filePaths);
    FileInfoData getFileInfo(int index) const;
    int count() const;
    void copyFrom(const FileStateSnapshot& other);

private:
    QList<FileInfoData> m_fileDataList;
};

#endif // FILESTATESNAPSHOT_H
