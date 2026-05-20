#ifndef QTSEXTRACTOR_H
#define QTSEXTRACTOR_H

#include "resource_copy.h"
#include <QDebug>
#include <QObject>
#include <QProcess>

class QtSExtractor : public QObject {
  Q_OBJECT
public:
  explicit QtSExtractor(QObject *parent = nullptr);
  int Process();

  QString fits_file, catalog_file; // Read only Input, and hard coded output
  QString sex_file, conv_file,
      param_file; // These files are resources and dynamically extracted
  float detect_thresh;
  float analysis_thresh;

  QString getFits_file() const;
  void setFits_file(const QString &newFits_file);

  QString getCatalog_file() const;
  void setCatalog_file(const QString &newCatalog_file);

  QString getSex_file() const;
  void setSex_file(const QString &newSex_file);

  QString getConv_file() const;
  void setConv_file(const QString &newConv_file);

  QString getParam_file() const;
  void setParam_file(const QString &newParam_file);

  float getDetect_thresh() const;
  void setDetect_thresh(float newDetect_thresh);

  float getAnalysis_thresh() const;
  void setAnalysis_thresh(float newAnalysis_thresh);

signals:
  void LoadResult(QString CatalogFile);

private:
  ResourceCopy *rc;
};

#endif // QTSEXTRACTOR_H
