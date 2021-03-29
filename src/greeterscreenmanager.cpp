#include "greeterscreenmanager.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include "greeterbackground.h"
#include "greeterloginwindow.h"

///析构:
///     清理登录窗口和背景窗口
GreeterScreenManager::~GreeterScreenManager ()
{
    if (m_greeterWindow != nullptr)
    {
        delete m_greeterWindow;
    }
    for (auto iter = m_BackgroundWidgetMap.begin(); iter != m_BackgroundWidgetMap.end();)
    {
        GreeterBackground *background = iter.value();
        delete background;
        background = nullptr;
        m_BackgroundWidgetMap.erase(iter++);
    }
}

///初始化窗口：
///     获取显示器数量，根据主显示器创建背景窗口，保存显示器和背景窗口的映射关系
///     创建GreeterWindow与主显示器绑定.若不存在主显示器，则不显示
void GreeterScreenManager::init ()
{
    for (QScreen *screen:qApp->screens())
    {
        //fix #36459,避免错误的数据,导致显示的问题
        if( screen->physicalSize().isEmpty() ){
            continue;
        }
        newScreenBackgroundWidget(screen);
    }
    m_greeterWindow = new GreeterLoginWindow();
    auto backgroundIter = m_BackgroundWidgetMap.find(qApp->primaryScreen());
    if (backgroundIter != m_BackgroundWidgetMap.end())
    {
        setGreeterOnBackground(backgroundIter.value());
    }
}

///屏幕添加：
///     创建新的背景窗口
///     如果登录窗口还未显示，则显示到新背景窗口上
void GreeterScreenManager::slotScreenAdded (QScreen *screen)
{
    newScreenBackgroundWidget(screen);
    if (m_greeterWindow->parent() == nullptr)
    {
        auto backgroundIter = m_BackgroundWidgetMap.find(qApp->primaryScreen());
        if (backgroundIter != m_BackgroundWidgetMap.end())
        {
            setGreeterOnBackground(backgroundIter.value());
        }
    }
}

///屏幕被移除:
///     删除和屏幕映射的背景窗口
///     更新窗口和屏幕映射关系
///     如果GreeterWindow显示在该窗口上移动到新的主显示器　调用GreeterWindow中的setScreen重新绑定屏幕
///     若主屏幕不存在不显示
void GreeterScreenManager::slotScreenRemoved (QScreen *screen)
{
    auto iter = m_BackgroundWidgetMap.find(screen);
    if (iter != m_BackgroundWidgetMap.end())
    {
        //隐藏窗口并延迟删除
        QObject *backgroundObject = dynamic_cast<QObject *>(iter.value());
        if (backgroundObject == m_greeterWindow->parent())
        {
            m_greeterWindow->setParent(nullptr);
        }
        iter.value()->hide();
        iter.value()->deleteLater();
        m_BackgroundWidgetMap.erase(iter);
    }

    if (m_greeterWindow->parent() == nullptr)
    {
        auto backgroundIter = m_BackgroundWidgetMap.find(qApp->primaryScreen());
        if (backgroundIter != m_BackgroundWidgetMap.end())
        {
            setGreeterOnBackground(backgroundIter.value());
        }
    }
}

///鼠标移入背景窗口槽函数:
///     移动GreeterWindow到背景窗口
void GreeterScreenManager::mouseEnterInWindow (GreeterBackground *background)
{
    if (m_greeterWindow->parent() != background)
    {
        setGreeterOnBackground(background);
    }
}

void GreeterScreenManager::newScreenBackgroundWidget (QScreen *screen)
{
    GreeterBackground *background = new GreeterBackground(screen);
    background->setObjectName(QString("LoginBackground_%1").arg(m_BackgroundWidgetMap.size()));
    m_BackgroundWidgetMap.insert(screen, background);
    connect(background, &GreeterBackground::mouseEnter,
            this, &GreeterScreenManager::mouseEnterInWindow);
    background->show();
}

void GreeterScreenManager::setGreeterOnBackground (GreeterBackground *background)
{
    m_greeterWindow->hide();
    m_greeterWindow->resize(background->width(), background->height());
    m_greeterWindow->setParent(background, Qt::X11BypassWindowManagerHint);
    m_greeterWindow->show();
    m_greeterWindow->setEditPromptFocus();
    m_greeterWindow->activateWindow();
}

GreeterScreenManager::GreeterScreenManager (QObject *parent)
        : QObject(parent), m_greeterWindow(nullptr)
{
    connect(qApp, &QApplication::screenAdded,
            this, &GreeterScreenManager::slotScreenAdded);
    connect(qApp, &QApplication::screenRemoved,
            this, &GreeterScreenManager::slotScreenRemoved);
}
