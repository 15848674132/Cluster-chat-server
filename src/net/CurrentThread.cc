#include "CurrentThread.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>


namespace CurrentThread
{
    __thread int t_cacheTid = 0;
    void cacheTid()
    {
        if(t_cacheTid == 0)
        {
            t_cacheTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}