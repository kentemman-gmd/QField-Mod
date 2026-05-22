#ifndef QFIELDSTORAGEMANAGER_H
#define QFIELDSTORAGEMANAGER_H
#include <QString>
class QFieldStorageManager
{
  public:
    static QFieldStorageManager &instance();
    QString projectsPath() const;
    QString backupPath() const;
    QString attachmentPath() const;
    QString cachePath() const;
    bool ensureDirectories() const;
    bool migrateLegacyProjects() const;

  private:
    QFieldStorageManager() = default;
    QString basePath() const;
    QString legacyBasePath() const;
};
#endif
