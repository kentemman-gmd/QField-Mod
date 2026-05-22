#include "qfieldstoragemanager.h"
#include "fileutils.h"
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#if defined( Q_OS_ANDROID )
#include <QCoreApplication>
#include <QJniObject>
#include <qnativeinterface.h>
#endif
QFieldStorageManager &QFieldStorageManager::instance()
{
  static QFieldStorageManager sManager;
  return sManager;
}
QString QFieldStorageManager::basePath() const
{
#if defined( Q_OS_ANDROID )
  auto *app = QCoreApplication::instance();
  if ( app )
  {
    QJniObject context( app->nativeInterface<QNativeInterface::QAndroidApplication>()->context() );
    if ( context.isValid() )
    {
      QJniObject jniPath = context.callObjectMethod<jstring>( "getQFieldDocumentsDirectory", "()Ljava/lang/String;" );
      if ( jniPath.isValid() && !jniPath.toString().isEmpty() )
        return jniPath.toString();
    }
  }
#endif
  return QStandardPaths::standardLocations( QStandardPaths::DocumentsLocation ).value( 0 ) + QStringLiteral( "/QField" );
}
QString QFieldStorageManager::legacyBasePath() const
{
#if defined( Q_OS_ANDROID )
  return QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
#else
  return QString();
#endif
}
QString QFieldStorageManager::projectsPath() const { return basePath(); }
QString QFieldStorageManager::backupPath() const { return basePath() + QStringLiteral( "/Backups" ); }
QString QFieldStorageManager::attachmentPath() const { return basePath() + QStringLiteral( "/Attachments" ); }
QString QFieldStorageManager::cachePath() const { return QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + QStringLiteral( "/cache" ); }
bool QFieldStorageManager::ensureDirectories() const
{
  QDir dir;
  return dir.mkpath( projectsPath() ) && dir.mkpath( backupPath() ) && dir.mkpath( attachmentPath() ) && dir.mkpath( projectsPath() + QStringLiteral( "/Imported Projects" ) ) && dir.mkpath( projectsPath() + QStringLiteral( "/Imported Datasets" ) ) && dir.mkpath( projectsPath() + QStringLiteral( "/Exports" ) );
}
bool QFieldStorageManager::migrateLegacyProjects() const
{
  QSettings settings;
  if ( settings.value( QStringLiteral( "storage/legacyMigrationDone" ), false ).toBool() )
    return true;
  const QString source = legacyBasePath();
  const QString destination = projectsPath();
  if ( source.isEmpty() || source == destination || !QFileInfo::exists( source ) )
  {
    settings.setValue( QStringLiteral( "storage/legacyMigrationDone" ), true );
    return true;
  }
  bool migrated = true;
  const QStringList folders{ QStringLiteral( "Imported Projects" ), QStringLiteral( "Imported Datasets" ), QStringLiteral( "DCIM" ), QStringLiteral( "Backups" ) };
  for ( const QString &folder : folders )
  {
    const QString sourceFolder = source + QLatin1Char( '/' ) + folder;
    if ( QFileInfo::exists( sourceFolder ) )
      migrated = migrated && FileUtils::copyRecursively( sourceFolder, destination + QLatin1Char( '/' ) + folder, true );
  }
  settings.setValue( QStringLiteral( "storage/legacyMigrationDone" ), migrated );
  return migrated;
}
