
#include "qtsextractor.h"
int main(int argc, char *argv[]) {
  // QApplication a(argc, argv);
  QtSExtractor *se = new QtSExtractor();
  se->setFits_file(argv[1]);
  if (!se->Process()) {
    qInfo() << "Process ran ok";
  } else {
    {
      qWarning() << "Process did not run as planned";
    }
  }

  // return a.exec();
  return 0;
}
