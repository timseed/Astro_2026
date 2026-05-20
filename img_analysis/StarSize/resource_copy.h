#pragma once
#include <QDir>
#include <QFile>
#include <QObject>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryDir>
#include <qtemporarydir.h>
class ResourceCopy : public QObject {
  Q_OBJECT
public:
  explicit ResourceCopy(QObject *parent = nullptr);
  ~ResourceCopy();
  int ExtractFile(QString resourcePath, QString &tempPathName);
  QString getTempDir();

private:
  QTemporaryDir *sexTmpDir;
  QList<QString> to_remove;
};
