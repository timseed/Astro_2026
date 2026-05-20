#include "resource_copy.h"
#include <qtemporarydir.h>

ResourceCopy::ResourceCopy(QObject *parent) : QObject(parent) {
  sexTmpDir = new QTemporaryDir();
  qInfo() << "Created temp dir called " << sexTmpDir->path();
}

ResourceCopy::~ResourceCopy() {
  for (const QString &f : to_remove) {
    qInfo() << "Removing tmp file " << f;
    QFile::remove(f);
  }
}

int ResourceCopy::ExtractFile(
    QString resource_name,
    QString &newTmp) { // Define resource and destination paths
  //
  // a resource name will look like :/dir/dir/file.ext
  //
  QFileInfo res_info(resource_name);
  QString tempFilePath = sexTmpDir->path() + "/" + res_info.fileName();
  qInfo() << "trying to place " << resource_name << " into " << tempFilePath;
  // Copy the file
  if (QFile::copy(resource_name, tempFilePath)) {
    // Optional: Ensure the file has execution permissions on Unix systems
    QFile::setPermissions(tempFilePath, QFileDevice::ReadOwner |
                                            QFileDevice::WriteOwner |
                                            QFileDevice::ExeOwner);
    newTmp = tempFilePath;
    return 0;
  } else {
    // Handle error (e.g., file already exists or write error)
    qWarning() << "Something went wrong tring to extract" << resource_name;
    return 1;
  }
}

QString ResourceCopy::getTempDir() { return sexTmpDir->path(); }
