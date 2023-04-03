/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */
#pragma once
#include <QGSettings>
#include <QVariant>
#include <QWidget>


QT_BEGIN_NAMESPACE
class QStackedWidget;
class QLabel;
class QComboBox;
class QLineEdit;
class QPushButton;
QT_END_NAMESPACE

class KiranSwitchButton;
class KiranSidebarWidget;
class KiranImageSelector;

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
struct GreeterSettingInfo
{
    struct GeneralSetting
    {
        QString background;
        bool hideUserList;
        bool allowManualLogin;
        quint64 scaleMode;
        quint64 scaleFactor;

        bool operator==(const GeneralSetting other) const
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
        QString autoLoginUser;
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

class HoverTips;
class Prefs;
class SettingWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SettingWindow(QWidget *parent = nullptr);
    ~SettingWindow();

    QSize sizeHint() const override;

private:
    void initUI();

    QWidget *initPageGeneralSettings();
    QWidget *initPageAutoLogin();

    void initUserComboBox(QComboBox *userComboBox);

    void saveGeneralSettings();
    void resetGeneralSettings();

    void saveAutoLoginSettings();
    void resetAutoLoginSettings();

    GreeterSettingInfo::GeneralSetting getGeneralSettingInfoFromBackend();
    GreeterSettingInfo::AutoLoginSetting getAutologinSettingInfoFromBackend();

private slots:
    void onScaleModeChanged(int idx);
    void onLoginOptionsChanged();

private:
    Prefs *m_prefs;
    GreeterSettingInfo m_origSettingInfo; /** < 缓存的配置信息,用于判断后端配置是否已经更改 */

    HoverTips *m_hoverTips; /** < 悬浮提示框 */

    KiranSidebarWidget *m_sidebarWidget;
    QStackedWidget *m_stackedWidget;

    KiranImageSelector *m_imageSelector; /** < 图片选择控件 **/
    QComboBox *m_comboScaleMode;         /** < 缩放下拉框 */
    QComboBox *m_comboScaleFactor;       /** < 缩放比例下拉框 */
    using SwitchButton = KiranSwitchButton;
    SwitchButton *m_enableManualSwitch; /** < 启用手动登录开关 */
    SwitchButton *m_showUserListSwitch; /** < 显示用户列表开关 */

    KiranSwitchButton *m_autologinSwitch; /** < 自动登录开关 */
    QWidget *m_autologinComboWidget;
    QComboBox *m_comboAutoLoginUser; /** < 自动登录用户下拉框 */
    QLineEdit *m_editAutoLoginDelay; /** < 自动登录延时输入框 */
};
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran