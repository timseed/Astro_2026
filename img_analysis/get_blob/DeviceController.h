#pragma once
#include "indi_ccd_cli.h"
#include <QDebug>
#include <QFile>
#include <QList>
#include <QObject>
#include <QString>
#include <libindi/baseclient.h>
#include <libindi/basedevice.h>

class DeviceController : public QObject {
  Q_OBJECT

public:
  explicit DeviceController(SimpleClient *client, QObject *parent = nullptr);
  QList<QString> Camera_Names;
private slots:
  void onDeviceConnected(QString device_name, INDI::BaseDevice dp);
  void onTemperatureChanged(double temp);
  void onImageReceived(const QByteArray &data);
  void onMessage(const QString &msg);

private:
  SimpleClient *m_client = nullptr;
  bool m_targetReached = false;

  void setTemperature(double value);
  void takeExposure(double seconds);
};
