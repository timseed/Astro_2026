#include "qtsextractor.h"
#include "resource_copy.h"

QtSExtractor::QtSExtractor(QObject *parent) : QObject{parent} {
  rc = new ResourceCopy();
  QString default_param_file;
  QString default_sex_file;
  QString default_conv_file;
  rc->ExtractFile(":/resource/default.param", default_param_file);
  rc->ExtractFile(":/resource/default.sex", default_sex_file);
  rc->ExtractFile(":/resource/default.conv", default_conv_file);

  setCatalog_file("output.cat");
  setSex_file(default_sex_file);
  setConv_file(default_conv_file);
  setParam_file(default_param_file);
  setDetect_thresh(2.0);
  setAnalysis_thresh(2.0);
}

int QtSExtractor::Process() {

  // Arguments for SExtractor
  QStringList args;
  args << getFits_file() << "-c" << getSex_file() << "-CATALOG_NAME"
       << getCatalog_file() << "-DETECT_THRESH"
       << QString::number(getDetect_thresh()) << "-ANALYSIS_THRESH"
       << QString::number(getAnalysis_thresh());
  qInfo() << "sex " << args;
  QProcess process;
  process.setWorkingDirectory(rc->getTempDir());
  qInfo() << "Process will run from " << rc->getTempDir();
  process.start("sex", args); // assumes 'sex' command is in PATH
  if (process.waitForFinished(-1)) {
    qDebug() << "Process finished successfully.";
    emit(LoadResult(getCatalog_file()));
  } else {
    qDebug() << "Process failed or timed out:" << process.errorString();
    return 1;
  }
  return 0;
}

float QtSExtractor::getAnalysis_thresh() const { return analysis_thresh; }

void QtSExtractor::setAnalysis_thresh(float newAnalysis_thresh) {
  analysis_thresh = newAnalysis_thresh;
}

float QtSExtractor::getDetect_thresh() const { return detect_thresh; }

void QtSExtractor::setDetect_thresh(float newDetect_thresh) {
  detect_thresh = newDetect_thresh;
}

QString QtSExtractor::getSex_file() const { return sex_file; }

void QtSExtractor::setSex_file(const QString &newSex_file) {
  sex_file = newSex_file;
}

QString QtSExtractor::getConv_file() const { return conv_file; }

void QtSExtractor::setConv_file(const QString &newConv_file) {
  conv_file = newConv_file;
}

QString QtSExtractor::getParam_file() const { return param_file; }

void QtSExtractor::setParam_file(const QString &newParam_file) {
  param_file = newParam_file;
}

QString QtSExtractor::getCatalog_file() const { return catalog_file; }

void QtSExtractor::setCatalog_file(const QString &newCatalog_file) {
  catalog_file = newCatalog_file;
}

QString QtSExtractor::getFits_file() const { return fits_file; }

void QtSExtractor::setFits_file(const QString &newFits_file) {
  fits_file = newFits_file;
}
