#include "DeviceController.h"
#include "basedevice.h"
#include "indidevapi.h"
#include <chrono>

DeviceController::DeviceController(SimpleClient *client, QObject *parent)
    : QObject(parent), m_client(client) {

  // Camera_Names << "Guide Simulator" << "CCD1" << "CCD2";
  Camera_Names << "SVBONY CCD SV405CC" << "SVBONY CCD SV905C2";
  connect(client, &SimpleClient::deviceConnected, this,
          &DeviceController::onDeviceConnected);

  // connect(client, &SimpleClient::ccdTemperatureChanged, this,
  //         &DeviceController::onTemperatureChanged);

  // connect(client, &SimpleClient::ccdImageReceived, this,
  //         &DeviceController::onImageReceived);

  // connect(client, &SimpleClient::indiMessage, this,
  //         &DeviceController::onMessage);
  // QObjectString server = "localhost";
  // int port = 7624;
  // setServer(server.toStdString().c_str(), port);
  client->connectServer();
  qInfo() << "Client Connected to the Server";
}

void DeviceController::onDeviceConnected(QString device_name,
                                         INDI::BaseDevice dp) {
  if (Camera_Names.contains(device_name)) {
    qInfo() << "Wanted device found " << device_name;
    // So we want to enable BLOBS
    m_client->setBLOBMode(BLOBHandling::B_ALSO,
                          device_name.toStdString().c_str());
    qInfo() << "*** Requested BLOB data from ***" << device_name;

  } else {

    qDebug() << "Device ignored (Qt layer) " << device_name;
  }
}

void DeviceController::onTemperatureChanged(double temp) {
  qDebug() << "CCD temperature:" << temp;

  if (qFuzzyCompare(temp + 1.0, -19.0)) {
    if (!m_targetReached) {
      m_targetReached = true;
      takeExposure(1.0);
    }
  }
}

void DeviceController::onImageReceived(const QByteArray &data) {
  QFile file("ccd_simulator.fits");

  if (file.open(QIODevice::WriteOnly)) {
    file.write(data);
    qDebug() << "Saved FITS image";
  } else {
    qWarning() << "Failed to write FITS file";
  }
}

void DeviceController::onMessage(const QString &msg) {
  qDebug() << "INDI message:" << msg;
}

void DeviceController::setTemperature(double value) {
  // m_client->setTemperatureINDI(value);
}

void DeviceController::takeExposure(double seconds) {
  // m_client->takeExposureINDI(seconds);
}
