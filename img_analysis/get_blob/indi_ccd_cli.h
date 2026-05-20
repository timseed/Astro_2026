#pragma once

#include <QCoreApplication>
#include <QDebug>
#include <libindi/baseclient.h>
#include <libindi/basedevice.h>
#include <unistd.h>

class SimpleClient : public QObject, public INDI::BaseClient {
  Q_OBJECT
public:
  SimpleClient();
  QString guider_images;
  QString main_images;
  int guide_cnt;
  int main_cnt;
signals:
  void deviceConnected(QString devicename, INDI::BaseDevice dp);

protected:
  // Called when a new device is defined in the INDI server
  void newDevice(INDI::BaseDevice dp) override;
  // Called when a property is updated (e.g., temperature, coordinates)
  void updateProperty(INDI::Property property) override;
  void newMessage(INDI::BaseDevice, int msg_id) override;
  void newBLOB(IBLOB *bp);
};
