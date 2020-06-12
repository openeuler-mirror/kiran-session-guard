#include "lockplug.h"
#include <QDebug>
#include <xcb/xcb.h>
#include "xlibhelper.h"
#include <iostream>
#include <QApplication>
#include <QX11Info>
#include <QWindow>
#include <QResizeEvent>

LockPlug::LockPlug(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("LockPlug");
    printID();
}

void LockPlug::printID()
{
    std::cout << "WINDOW ID=" << winId() << std::endl;
}

void LockPlug::responseOkAndQuit()
{
    std::cout << "RESPONSE=OK" << std::endl;
    this->close();
}

void LockPlug::responseCancelAndQuit()
{
    std::cout << "RESPONSE=CANCEL" << std::endl;
    this->close();
}

void LockPlug::responseNoticeAuthFailed()
{
    std::cout << "NOTICE=AUTH FAILED" << std::endl;

}
