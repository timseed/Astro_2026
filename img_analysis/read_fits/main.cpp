#include "MyFits.h"
#include "count_stars.h"
#include <QCoreApplication>
#include <QDebug>
#include <fitsio.h>
#include <qlogging.h>

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  if (argc != 2) {
    qWarning() << "Expected read_fits <filename>";
    qWarning() << "read_fits closing";
    exit(0);
  }
  MyFits *mf = new MyFits();
  mf->setFilename(argv[1]);
  if (mf->read()) {
    qWarning() << "Problem reading the file " << mf->getFilename();
    exit(0);
  }
  qInfo() << "We had read file... now try and extact some Star positions";
  ImageStats star_stats;
  star_stats = calculateQtStats(mf->getPixels());
  qInfo() << "We got Mean Size of " << star_stats.mean << " and a SD of "
          << star_stats.stdDev;
  qInfo() << "Star Count is "
          << countStarsQt(mf->getPixels(), mf->getWidth(), mf->getHeight());
  return 0;
}
