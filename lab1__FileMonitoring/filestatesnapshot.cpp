#include "filestatesnapshot.h"
#include <QFileInfo>

FileStateSnapshot::FileStateSnapshot() { }

void FileStateSnapshot::populate(const QStringList& filePaths) {
    m_fileDataList.clear();
    m_fileDataList.reserve(filePaths.size());
    for (const QString& path : filePaths) {
        QFileInfo qfi(path);
        qfi.refresh();
        m_fileDataList.append(FileInfoData(path, qfi.exists(), qfi.exists() ? qfi.size() : -1));
    }
}

FileInfoData FileStateSnapshot::getFileInfo(int index) const {
    if (index >= 0 && index < m_fileDataList.size()) {
        return m_fileDataList.at(index);
    }
    return FileInfoData();
}

int FileStateSnapshot::count() const {
    return m_fileDataList.size();
}

void FileStateSnapshot::copyFrom(const FileStateSnapshot& other) {
    this->m_fileDataList = other.m_fileDataList;
}
