#ifndef GREETERSETTING_H
#define GREETERSETTING_H

#include <kiranwidgets-qt5/kiran-titlebar-window.h>
#include <QGSettings>
#include <QVariant>
#include <QWidget>

struct GreeterSettingInfo
{
    struct AppearanceSetting
    {
        QString background;
        bool    hideUserList;
        bool    allowManualLogin;
        quint64 scaleMode;
        quint64 scaleFactor;

        bool operator==(const AppearanceSetting other) const
        {
            if (this->background == other.background &&
                this->hideUserList == other.hideUserList &&
                this->allowManualLogin == other.allowManualLogin &&
                this->scaleMode == other.scaleMode &&
                this->scaleFactor == other.scaleFactor)
            {
                return true;
            }
            return false;
        }
    } appearanceInfo;

    struct AutoLoginSetting
    {
        QString  autoLoginUser;
        uint64_t autoLoginTimeout;

        bool operator==(const AutoLoginSetting other) const
        {
            if (this->autoLoginUser == other.autoLoginUser &&
                this->autoLoginTimeout == other.autoLoginTimeout)
            {
                return true;
            }
            return false;
        }
    } autoLoginInfo;

    bool operator==(const GreeterSettingInfo other) const
    {
        if (this->appearanceInfo == other.appearanceInfo && this->autoLoginInfo == other.autoLoginInfo)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

class KiranSwitchButton;
class KiranSidebarWidget;
class QStackedWidget;
class QLabel;
class QComboBox;
class QLineEdit;
class QPushButton;
class HoverTips;
class KiranImageSelector;
class GreeterSettingWindow : public KiranTitlebarWindow
{
    Q_OBJECT
public:
    explicit GreeterSettingWindow(QWidget* parent = nullptr);
    ~GreeterSettingWindow();

private:
    void initUI();

    QWidget *initPageAppearance();
    QWidget *initPageAutoLogin();

    void initUserComboBox(QComboBox *userComboBox);

    void saveAppearanceSettings();
    void resetAppearanceSettings();

    void saveAutoLoginSettings();
    void resetAutoLoginSettings();

    GreeterSettingInfo::AppearanceSetting getAppearanceSettingInfoFromBackend();
    GreeterSettingInfo::AppearanceSetting getAppearanceSettingInfoFromUI();

    GreeterSettingInfo::AutoLoginSetting getAutologinSettingInfoFromBackend();
    GreeterSettingInfo::AutoLoginSetting getAutologinSettingInfoFromUI();

private:
    GreeterSettingInfo m_origSettingInfo; /** < 缓存的配置信息,用于判断后端配置是否已经更改 */

    HoverTips *m_hoverTips; /** < 悬浮提示框 */

    KiranSidebarWidget *m_sidebarWidget;
    QStackedWidget *    m_stackedWidget;

    KiranImageSelector *m_imageSelector;    /** < 图片选择控件 **/
    QComboBox *         m_comboScaleMode;   /** < 缩放下拉框 */
    QComboBox *         m_comboScaleFactor; /** < 缩放比例下拉框 */
    using SwitchButton = KiranSwitchButton;
    SwitchButton *m_enableManualSwitch; /** < 启用手动登录开关 */
    SwitchButton *m_hideUserListSwitch; /** < 隐藏用户列表开关 */

    QComboBox *m_comboAutoLoginAccount; /** < 自动登录账户下拉框 */
    QLineEdit *m_editAutoLoginDelay;    /** < 自动登录延时输入框 */
};

#endif  // GREETERSETTING_H
