#ifndef TOOL_H
#define TOOL_H

#include <QImage>

namespace Tool {
    QImage blurredImage(const QImage& image, const QRect& rect, int radius, bool alphaOnly);
}

#endif // TOOL_H
