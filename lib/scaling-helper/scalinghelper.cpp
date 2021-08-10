#include <qt5-log-i.h>
#include <stdlib.h>
#include <QtMath>
#include <list>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include "scalinghelper.h"


void ScalingHelper::set_scale_factor (double factor)
{
    qputenv("QT_SCALE_FACTOR", QString::number(factor).toUtf8());
    KLOG_INFO() << "scale factor:" << factor;
}

void ScalingHelper::auto_calculate_screen_scaling ()
{
    Display *display = nullptr;
    XRRScreenResources *resources = nullptr;
    qreal scale_factor = 1.0;
    std::list<double> scaleFactors;

    display = XOpenDisplay(nullptr);
    resources = XRRGetScreenResourcesCurrent(display, DefaultRootWindow(display));

    if (!resources)
    {
        KLOG_WARNING("XRRGetScreenResourcesCurrent failed,try XRRGetScreenResources");
        resources = XRRGetScreenResources(display, DefaultRootWindow(display));
    }

    if (!resources)
    {
        KLOG_WARNING("get screen resources failed");
        goto failed;
    }

    if (resources)
    {
        for (int i = 0; i < resources->noutput; i++)
        {
            XRROutputInfo *outputInfo = XRRGetOutputInfo(display, resources, resources->outputs[i]);
            if (outputInfo->crtc == 0 || outputInfo->mm_width == 0)
            {
                if (outputInfo != nullptr)
                {
                    XRRFreeOutputInfo(outputInfo);
                }
                continue;
            }
            XRRCrtcInfo *crtInfo = XRRGetCrtcInfo(display, resources, outputInfo->crtc);
            if (crtInfo == nullptr)
            {
                XRRFreeOutputInfo(outputInfo);
                continue;
            }

            //计算屏幕尺寸
            qreal screenInch;
            screenInch = qSqrt(qPow(outputInfo->mm_width, 2.0) + qPow(outputInfo->mm_height, 2.0)) / qreal(25.4);

            //计算ppi
            qreal hypotenusePixel = qSqrt(qPow(crtInfo->width, 2.0) + qPow(crtInfo->height, 2.0));
            qreal ppi = hypotenusePixel / screenInch;

            KLOG_INFO() << "Screen:" << outputInfo->name;
            KLOG_INFO() << "    physical size:   " << outputInfo->mm_width << "x" << outputInfo->mm_height;
            KLOG_INFO() << "    virtual size:    " << crtInfo->width << crtInfo->height;
            KLOG_INFO() << "    inch:            " << screenInch;
            KLOG_INFO() << "    ppi:             " << ppi;

            double screen_scale_factor = 1.0;
            if (ppi >= 150 && ppi < 196)
            {
                screen_scale_factor = 1.5;
            }
            else if (ppi >= 196)
            {
                screen_scale_factor = 2.0;
            }
            else if (crtInfo->width >= 4096)
            {
                screen_scale_factor = 2.0;
            }
            scaleFactors.push_back(screen_scale_factor);
            XRRFreeCrtcInfo(crtInfo);
            XRRFreeOutputInfo(outputInfo);
        }
        XRRFreeScreenResources(resources);
    }
    XCloseDisplay(display);
    if (scaleFactors.size())
    {
        scaleFactors.sort();
        scale_factor = *scaleFactors.begin();
    }
    KLOG_INFO() << "QT_SCALE_FACTOR:" << scale_factor;
    if (!qputenv("QT_SCALE_FACTOR", QString::number(scale_factor).toUtf8()))
    {
        KLOG_WARNING() << "set scale factor failed.";
    }
    return;
    failed:
    if (display)
    {
        XCloseDisplay(display);
    }
    KLOG_WARNING("auto_calculate_screen_scaling failed,set QT_AUTO_SCREEN_SCALE_FACTOR=1");
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
}
