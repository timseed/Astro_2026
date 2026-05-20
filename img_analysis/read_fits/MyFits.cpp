#include "MyFits.h"

bool MyFits::read() {
  fitsfile *fptr; // Pointer to the FITS file
  int status = 0; // CFITSIO status must be initialized to 0
  int bitpix, naxis;
  long naxes[2] = {1, 1};

  // 1. Open the file
  QByteArray fileArray = filename.toUtf8(); // Convert to UTF-8 QByteArray
  const char *file_cstr = fileArray.constData();
  if (fits_open_file(&fptr, file_cstr, READONLY, &status)) {
    qCritical() << "Error opening file:" << status;
    return status;
  }

  // 2. Move to the primary HDU (or the first image extension)
  // FITS images can be in the primary array or an extension
  int hdutype;
  fits_movabs_hdu(fptr, 1, &hdutype, &status);

  // 3. Get image dimensions
  fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status);

  setWidth(naxes[0]);
  setHeight(naxes[1]);
  long totalPixels = getWidth() * getHeight();

  qDebug() << "Image Dimensions:" << width << "x" << height;
  qDebug() << "Total Pixels:" << totalPixels;

  // 4. Read the data
  // We'll use double to be safe, but you can use float or int
  pixels = QVector<double>(totalPixels);
  long fpixel[2] = {1, 1}; // FITS is 1-indexed

  fits_read_pix(fptr, TDOUBLE, fpixel, totalPixels, nullptr, pixels.data(),
                nullptr, &status);

  if (status) {
    qCritical() << "Error reading pixels:" << status;
  } else {
    qDebug() << "Successfully read the first pixel value:" << pixels[0];
  }

  // 5. Close the file
  fits_close_file(fptr, &status);
  return status;
}
