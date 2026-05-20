#pragma once
#include <QCoreApplication>
#include <QPoint>
#include <QQueue>
#include <QVector>
#include <QtConcurrent> // For faster math on large images
#include <QtMath>

struct ImageStats {
  double mean;
  double stdDev;
};

// Uses Qt Concurrent to find mean/stdDev quickly
ImageStats calculateQtStats(const QVector<double> &pixels);
int countStarsQt(const QVector<double> &pixels, int width, int height);
