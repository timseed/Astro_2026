#include "indi_ccd_cli.h"
#include "baseclient.h"
#include <QFile>
#include <QImage>
#include <QObject>
#include <format>

SimpleClient::SimpleClient() : QObject(), INDI::BaseClient() {
  guide_cnt = 1;
  main_cnt = 1;
}
void SimpleClient::newDevice(INDI::BaseDevice dp) {
  // qInfo() << "indi_ccd_cli New Device: " << dp.getDeviceName();
  emit deviceConnected(dp.getDeviceName(), dp);
}

// Called when a property is updated (e.g., temperature, coordinates)
void SimpleClient::updateProperty(INDI::Property property) {
  if (property.getType() == INDI_NUMBER &&
      property.isNameMatch("CCD_TEMPERATURE")) {
    auto temp = property.getNumber()->at(0)->getValue();
    qInfo() << "Temperature updated: " << temp;
  }
  if (property.getType() == INDI_BLOB) {
    qInfo() << "Got BLOB";
    auto *blobProp = property.getBLOB(); // Get the BLOB property vector
    for (auto &bp : *blobProp) {
      QString filename;
      if (bp.size < 3000000) {
        // Your existing logic for saving the FITS file goes here:
        filename = QString("Guider_%1.fits").arg(guide_cnt);
        ++guide_cnt;
      } else {
        filename = QString("main_%1.fits").arg(main_cnt);
        ++main_cnt;
      }
      QFile file(filename);
      if (file.open(QIODevice::WriteOnly)) {
        file.write(reinterpret_cast<const char *>(bp.blob), bp.size);
        file.close();
        qInfo() << "IndiBlob: Created file:" << filename;
      }
    }
  }
}

// Called when server sends a message
void SimpleClient::newMessage(INDI::BaseDevice dp, int messageID) {
  qInfo() << "Message: " << dp.messageQueue(messageID);
}

void SimpleClient::newBLOB(IBLOB *bp) {
  // Access property details
  // const char* deviceName = bp->pg->device;
  // const char* propertyName = bp->pg->name;
  // 1. Get the raw data from INDI
  // bp->blob is the pointer to the data, bp->size is the length in bytes
  QByteArray rawData = QByteArray::fromRawData(
      reinterpret_cast<const char *>(bp->blob), bp->size);
  qInfo() << "We got an Image size " << bp->size;
  // Rough Maths
  // SV905C is aprox 1280x960 pixels ... or 1.2 MegaPixels
  // Lets add some Padding so <2Mb it is a guide Image ... more it is a guide
  // image
  // 2. Handle the data (e.g., save it or load it into an image)
  if (QString(bp->format) == ".fits") {
    if (bp->size > 9000000) {
      qInfo() << "Main Camera assumed as image size is " << bp->size;
    } else {
      qInfo() << "Guide Camera assumed as image size is " << bp->size;
      QString oFile = QString("Guide%1.fits").arg(guide_cnt);
      QFile file(oFile);

      // 3. Open the file in WriteOnly mode
      if (file.open(QIODevice::WriteOnly)) {
        // 4. Write the raw binary data directly from the INDI pointer
        qint64 bytesWritten =
            file.write(reinterpret_cast<const char *>(bp->blob), bp->size);

        if (bytesWritten == bp->size) {
          qDebug() << "Successfully saved FITS file:" << oFile;
        } else {
          qWarning() << "Failed to write complete file. Wrote" << bytesWritten
                     << "of" << bp->size;
        }

        file.close(); // Always close to flush the buffer to disk
      } else {
        qCritical() << "Could not open file for writing:" << file.errorString();
      }
    }
  } else {
    qWarning() << "Image received was not FITS but " << bp->format
               << " It has NOT been Saved";
  }
}
