#ifndef IMAGEHELPER_H
#define IMAGEHELPER_H

#include <QImage>

namespace ImageHelper{
    QImage blurredImage(const QImage& image, const QRect& rect, int radius, bool alphaOnly);
}

#endif // IMAGEHELPER_H
