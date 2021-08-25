//
// Created by lxh on 2021/8/4.
//

#ifndef KIRAN_SCREENSAVER_DIALOG_SRC_PREFS_H_
#define KIRAN_SCREENSAVER_DIALOG_SRC_PREFS_H_

class Prefs
{
public:
    static Prefs* instance();
    ~Prefs();

    bool canPowerOff();
    bool canReboot();
    bool canSuspend();

private:
    Prefs();

private:
    bool m_canPowerOff = true;
    bool m_canReboot = true;
    bool m_canSuspend = true;
};

#endif  //KIRAN_SCREENSAVER_DIALOG_SRC_PREFS_H_
