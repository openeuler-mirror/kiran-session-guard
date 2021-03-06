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

#ifndef GREETERSETTING_H
#define GREETERSETTING_H

#include <QGSettings>
#include <QVariant>
#include <QWidget>

struct GreeterSettingInfo
{
    struct GeneralSetting
    {
        QString background;
        bool    hideUserList;
        bool    allowManualLogin;
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
class GreeterSettingWindow : public QWidget
{
    Q_OBJECT
public:
    explicit GreeterSettingWindow(QWidget* parent = nullptr);
    ~GreeterSettingWindow();

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
    GreeterSettingInfo m_origSettingInfo; /** < ?????????????????????,?????????????????????????????????????????? */

    HoverTips *m_hoverTips; /** < ??????????????? */

    KiranSidebarWidget *m_sidebarWidget;
    QStackedWidget *    m_stackedWidget;

    KiranImageSelector *m_imageSelector;    /** < ?????????????????? **/
    QComboBox *         m_comboScaleMode;   /** < ??????????????? */
    QComboBox *         m_comboScaleFactor; /** < ????????????????????? */
    using SwitchButton = KiranSwitchButton;
    SwitchButton *m_enableManualSwitch; /** < ???????????????????????? */
    SwitchButton *m_showUserListSwitch; /** < ???????????????????????? */

    QComboBox *m_comboAutoLoginUser; /** < ??????????????????????????? */
    QLineEdit *m_editAutoLoginDelay;    /** < ??????????????????????????? */
};

#endif  // GREETERSETTING_H
