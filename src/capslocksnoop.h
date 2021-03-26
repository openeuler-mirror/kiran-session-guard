#ifndef CAPSLOCKSNOOP_H
#define CAPSLOCKSNOOP_H

#include <string>
#include <list>

typedef void (*capslock_status_change_callback) (bool isOn, void *user_data);
typedef struct _CapsLockSnoopPrivate CapsLockSnoopPrivate;

/**
 * @brief CapsLock简单全局监控
 */
class CapsLockSnoop
{
public:
    CapsLockSnoop ();
    ~CapsLockSnoop ();
    bool start (capslock_status_change_callback callback, void *user_data, std::string &error);
    bool stop ();
private:
    bool getCapsLockCurrentState (bool &isOn, std::string &error);
    static void *thread_record_func (void *param);
public:
    //NOTE:只是为了在回调record_intercept_proc_callback中能访问到
    CapsLockSnoopPrivate *m_private;
};

#endif // CAPSLOCKSNOOP_H
