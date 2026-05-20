#/bin/bash
QT_ROOT=$(qmake6 -query QT_INSTALL_PREFIX)
echo "QT is based at $QT_ROOT"
cmake \
  -DQt6_DIR=$QT_ROOT/lib/cmake/Qt6 \
  -DUSE_QT6=ON \
  -DBUILD_WITH_QT6=ON \
  -DQT_DEBUG_FIND_PACKAGE=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DBUILD_TESTING=OFF \
  ..
