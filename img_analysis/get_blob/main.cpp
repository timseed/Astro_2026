#include "DeviceController.h"
#include "indi_ccd_cli.h"
#include <QCoreApplication>
#include <QDebug>
#include <libindi/baseclient.h>
#include <libindi/basedevice.h>
#include <unistd.h>
int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);
  SimpleClient client;
  DeviceController dc(&client);

  // Connect to local or remote INDI server (default port is 7624)
  client.setServer("localhost", 7624);

  if (client.connectServer()) {
    qInfo() << "Connected to INDI server. Waiting for events...";
    // Keep the main thread alive to listen for updates
  }

  // -----------------------------
  // 4. Start event loop
  // -----------------------------
  return app.exec();
}
