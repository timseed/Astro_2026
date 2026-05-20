#include <QApplication>
// Includes for this project
#include "binning.h"
#include "ssolverutils/fileio.h"
#include "stellarsolver.h"
#include "structuredefinitions.h"
int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
#if defined(__linux__)
  setlocale(LC_NUMERIC, "C");
#endif
  fileio fileLoader;
  fileLoader.logToSignal = false;
  if (!fileLoader.loadImage(argv[1])) {
    printf("Error in loading FITS file 1st parameter ");
    exit(1);
  }
  FITSImage::Statistic stats = fileLoader.getStats();
  uint8_t *imageBuffer = fileLoader.getImageBuffer();

  StellarSolver stellarSolver(stats, imageBuffer);
  stellarSolver.setProperty("ExtractorType", SSolver::EXTRACTOR_INTERNAL);
  stellarSolver.setProperty("ProcessType", SSolver::EXTRACT_WITH_HFR);
  stellarSolver.setParameterProfile(SSolver::Parameters::ALL_STARS);

  if (!stellarSolver.extract(true)) {
    printf("Solver Failed");
    exit(0);
  }
  QList<FITSImage::Star> starList = stellarSolver.getStarList();
  QVector<double> hfr_vec;
  printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
  printf("Stars found: %u\n", starList.count());
  for (int i = 0; i < starList.count(); i++) {
    FITSImage::Star star = starList.at(i);
    printf("Star #%u: (%f x, %f y), (%f a, %f b, %f theta), mag: %f, flux: %f, "
           "peak: %f hfr: %f \n ",
           i, star.x, star.y, star.a, star.b, star.theta, star.mag, star.flux,
           star.peak, star.HFR);
    hfr_vec.append(star.HFR);
  }
  bin_values(hfr_vec);
  return 0;
}
