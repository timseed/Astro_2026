#include "binning.h"
int bin_values(QVector<double> values, int numBins) {
  if (values.isEmpty())
    return 1;

  // Find min/max
  auto [minIt, maxIt] = std::minmax_element(values.begin(), values.end());

  double minValue = *minIt;
  double maxValue = *maxIt;

  double binWidth = (maxValue - minValue) / numBins;

  // Prevent divide-by-zero if all values identical
  if (binWidth == 0.0)
    binWidth = 1.0;

  QVector<int> bins(numBins, 0);

  // Fill bins
  for (double v : values) {
    int index = static_cast<int>((v - minValue) / binWidth);

    // Handle max value edge case
    if (index >= numBins)
      index = numBins - 1;

    bins[index]++;
  }
  qDebug()
      << "=======================Binning==================================";
  // Display results
  for (int i = 0; i < numBins; ++i) {
    double start = minValue + i * binWidth;
    double end = start + binWidth;

    qDebug() << QString("[%1 - %2] : %3")
                    .arg(start, 0, 'f', 2)
                    .arg(end, 0, 'f', 2)
                    .arg(bins[i]);
  }
  qDebug()
      << "=======================Binning end==============================";
  return (0);
}
