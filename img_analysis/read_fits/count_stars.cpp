#include "count_stars.h"

// Uses Qt Concurrent to find mean/stdDev quickly
ImageStats calculateQtStats(const QVector<double>& pixels) {
    if (pixels.isEmpty()) return {0, 0};

    // Calculate Mean
    double sum = std::accumulate(pixels.begin(), pixels.end(), 0.0);
    double mean = sum / pixels.size();

    // Calculate Variance (Standard Deviation Squared)
    double sq_sum = 0;
    for(double p : pixels) {
        sq_sum += (p - mean) * (p - mean);
    }
    double stdDev = qSqrt(sq_sum / pixels.size());

    return {mean, stdDev};
}

int countStarsQt(const QVector<double>& pixels, int width, int height) {
    ImageStats stats = calculateQtStats(pixels);
    
    // 5-Sigma threshold is standard for "clearly visible" stars
    double threshold = stats.mean + (5 * stats.stdDev);
    
    int starCount = 0;
    QVector<bool> visited(pixels.size(), false);
    
    // Neighborhood offsets (8-way connectivity)
    const QVector<QPoint> neighbors = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},          {1,  0},
        {-1,  1}, {0,  1}, {1,  1}
    };

    for (int i = 0; i < pixels.size(); ++i) {
        if (pixels[i] > threshold && !visited[i]) {
            starCount++;
            
            // Start a Breadth-First Search (BFS) to mark this star's pixels
            QQueue<int> queue;
            queue.enqueue(i);
            visited[i] = true;

            while (!queue.isEmpty()) {
                int currIdx = queue.dequeue();
                int currX = currIdx % width;
                int currY = currIdx / width;

                for (const QPoint& d : neighbors) {
                    int nx = currX + d.x();
                    int ny = currY + d.y();
                    int nIdx = ny * width + nx;

                    if (nx >= 0 && nx < width && ny >= 0 && ny < height 
                        && !visited[nIdx] && pixels[nIdx] > threshold) {
                        visited[nIdx] = true;
                        queue.enqueue(nIdx);
                    }
                }
            }
        }
    }
    return starCount;
}
