#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QVector>
#include <fitsio.h>
class MyFits : public QObject {
public:
  explicit MyFits(QObject *parent = nullptr) : QObject(parent) {
    qInfo() << "Constrctor for MyFits";
  }
  bool read();
  void setFilename(const QString newfile) { filename = newfile; };
  const QString getFilename() { return filename; };
  QVector<double> getPixels() { return pixels; }
  long setWidth(long w) {
    width = w;
    return width;
  }
  long getWidth() { return width; }
  long setHeight(long h) {
    height = h;
    return height;
  }
  long getHeight() { return height; }

public:
  QString filename;
  QVector<double> pixels;
  long width;
  long height;
};
