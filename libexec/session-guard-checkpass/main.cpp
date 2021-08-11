#include <fcntl.h>
#include <qt5-log-i.h>
#include <security/pam_appl.h>
#include <sys/mman.h>
#include <iostream>
#include "pam-message.h"

#define PAM_SERVICE_NAME "mate-screensaver"

pam_handle_t *pamh = nullptr;
int CHANNEL_READ = 0;
int CHANNEL_WRITE = 0;

int conversation(int num_msg, const pam_message **msgs, pam_response **resp, void *appdata_ptr)
{
    struct pam_response *reply = NULL;
    char *username = nullptr;

    reply = (struct pam_response *)calloc(num_msg, sizeof(*reply));
    if (reply == nullptr)
    {
        KLOG_ERROR() << "can't malloc memory for replies,return PAM_CONV_ERR";
        return PAM_CONV_ERR;
    }

    pam_get_item(pamh, PAM_USER, (const void **)&username);

    int replyRet;
    replyRet = PAM_SUCCESS;
    for (int i = 0; i < num_msg && replyRet == PAM_SUCCESS; i++)
    {
        const pam_message *msg = msgs[i];
        switch (msg->msg_style)
        {
        case PAM_PROMPT_ECHO_OFF:
        case PAM_PROMPT_ECHO_ON:
        {
            PromptRequestEvent event(msg->msg_style == PAM_PROMPT_ECHO_OFF, msg->msg);

            if (!kiran_pam_message_send_event(CHANNEL_WRITE, &event))
            {
                //发送消息失败
                KLOG_DEBUG() << "send pam message to parent process failed!";
                replyRet = PAM_CONV_ERR;
            }
            else
            {
                PamEvent *recvReply = nullptr;
                //接收消息失败
                if (!kiran_pam_message_recv_event(CHANNEL_READ, &recvReply))
                {
                    KLOG_ERROR() << "recv pam prompt reply failed";
                    replyRet = PAM_CONV_ERR;
                }
                else
                {
                    //消息类型不正确
                    if (recvReply->type() != PamEvent::PromptReply)
                    {
                        KLOG_ERROR() << "recv event is not reply!";
                        replyRet = PAM_CONV_ERR;
                    }
                    else
                    {
                        auto replyEvent = dynamic_cast<PromptReplyEvent *>(recvReply);
                        KLOG_DEBUG() << "recv prompt reply:" << replyEvent->result() << replyEvent->text();
                        //消息返回失败
                        if (!replyEvent->result())
                        {
                            replyRet = PAM_CONV_ERR;
                        }
                        else
                        {
                            reply[i].resp_retcode = PAM_SUCCESS;
                            reply[i].resp = (char *)calloc(1, replyEvent->text().length() + 1);
                            strcpy(reply[i].resp, replyEvent->text().toStdString().c_str());
                        }
                    }
                }
            }
            break;
        }
        case PAM_ERROR_MSG:
        case PAM_TEXT_INFO:
        {
            MessageEvent messageEvent(msg->msg_style == PAM_ERROR_MSG, msg->msg);
            if (!kiran_pam_message_send_event(CHANNEL_WRITE, &messageEvent))
            {
                //发送消息失败
                KLOG_DEBUG() << "send pam message to parent process failed!";
                replyRet = PAM_CONV_ERR;
            }
        }
        default:
            break;
        }

        if (replyRet == PAM_SUCCESS)
        {
            reply[i].resp_retcode = PAM_SUCCESS;
        }
        else
        {
            for (int j = 0; j <= i; j++)
            {
                if (reply[i].resp != nullptr)
                    free(reply[i].resp);
            }
            free(reply);
        }
    }
    *resp = reply;
    return replyRet;
}

void no_fail_delay(int status, unsigned int delay, void *appdata_ptr)
{
}

int main(int argc, char *argv[])
{
    klog_qt5_init("",
                  "kylinsec-session",
                  "kiran-screensaver-dialog",
                  "kiran-session-guard-checkpass");

    ///判断参数合法性
    if (argc != 3)
    {
        KLOG_ERROR() << "usage: pam-authproxy-checkpass READFD WRITEFD USERNAME";
        for (int i = 0; i < argc; i++)
        {
            KLOG_DEBUG() << i << argv[i];
        }
        return EXIT_FAILURE;
    }

    CHANNEL_READ = atoi(argv[0]);
    CHANNEL_WRITE = atoi(argv[1]);
    QString userName = argv[2];

    if (CHANNEL_READ == 0 || CHANNEL_WRITE == 0)
    {
        KLOG_ERROR("invalid file descriptors: read %d write %d", CHANNEL_READ, CHANNEL_WRITE);
        return EXIT_FAILURE;
    }

    ///安全性
    //可以参考lightdm session-child
    /* Protect memory from being paged to disk, as we deal with passwords */
    mlockall(MCL_CURRENT | MCL_FUTURE);

    /* Don't let these pipes leak to the command we will run */
    fcntl(CHANNEL_READ, F_SETFD, FD_CLOEXEC);
    fcntl(CHANNEL_WRITE, F_SETFD, FD_CLOEXEC);

    KLOG_DEBUG() << "start checkpass child process:"
                 << "\n"
                 << "\t read fd:       " << CHANNEL_READ << "\n"
                 << "\t write fd:      " << CHANNEL_WRITE << "\n"
                 << "\t authproxy user name:" << userName;

    ///开始pam认证
    // clang-format off
    struct pam_conv conv = {
        .conv = &conversation,
        .appdata_ptr = nullptr
    };
    // clang-format on

    int ret = pam_start(PAM_SERVICE_NAME, userName.toStdString().c_str(), &conv, &pamh);
    if (ret != PAM_SUCCESS)
    {
        KLOG_WARNING() << "failed to start pam:" << pam_strerror(pamh, ret);
        return EXIT_FAILURE;
    }

    pam_set_item(pamh, PAM_FAIL_DELAY, (void *)no_fail_delay);

    int authRes = PAM_SUCCESS;
    authRes = pam_authenticate(pamh, 0);

    const char *newUserName;
    if (pam_get_item(pamh, PAM_USER, (const void **)&newUserName) != PAM_SUCCESS)
    {
        pam_end(pamh, 0);
        return EXIT_FAILURE;
    }

    const char *authResultString = pam_strerror(pamh, authRes);
    CompleteEvent event(true, authRes == PAM_SUCCESS, QString(authResultString));
    kiran_pam_message_send_event(CHANNEL_WRITE, &event);

    KLOG_DEBUG() << "checkpass child process exit.";
    return EXIT_SUCCESS;
}