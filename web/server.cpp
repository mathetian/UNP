#include "server.h"

void pr_cpu_time()
{
    double user, sys;

    struct rusage myusage, childusage;

    if(getrusage(RUSAGE_SELF, &myusage) < 0) err_sys("getrusage error");
    if(getrusage(RUSAGE_CHILDREN, &childusage) < 0) err_sys("getrusage error");

    user = (double)myusage.ru_utime.tv_sec + myusage.ru_utime.tv_usec/1000000.0;
    user += (double)childusage.ru_utime.tv_sec + childusage.ru_utime.tv_usec/1000000.0;
    sys = (double)myusage.ru_stime.tv_sec + myusage.ru_stime.tv_usec/1000000.0;
    sys += (double)childusage.ru_stime.tv_sec + childusage.ru_stime.tv_usec/1000000.0;

    printf("\nuser time = %g, sys time = %g\n", user, sys);
}

void web_child(int sockfd)
{
    int			ntowrite;
    ssize_t		nread;
    char		line[MAXLINE], result[MAXN];

    while(true)
    {
        if((nread = Readline(sockfd, line, MAXLINE)) == 0)
            return;

        ntowrite = atol(line);
        if((ntowrite <= 0) || (ntowrite > MAXN))
            err_quit("client request for %d bytes", ntowrite);

        Writen(sockfd, result, ntowrite);
    }
}

struct flock lock_it, unlock_it;
int    lock_fd = -1;

void my_lock_wait()
{
    int rc;
    while((rc = fcntl(lock_fd, F_SETLKW, &lock_it)) < 0)
    {
        if(errno == EINTR) continue;
        else err_sys("fcntl error for my_lock_wait");
    }
}

void my_lock_release()
{
    if(fcntl(lock_fd, F_SETLKW, &unlock_it) < 0)
        err_sys("fcntl error for my_lock_release");
}

void my_lock_init(const char *pathname)
{
    char lock_file[1024];
    strncpy(lock_file, pathname, sizeof(lock_file));
    lock_fd = mkstemp(lock_file);
    unlink(lock_file);

    lock_it.l_type = F_WRLCK;
    lock_it.l_whence = SEEK_SET;
    lock_it.l_start = 0;
    lock_it.l_len = 0;

    unlock_it.l_type = F_UNLCK;
    unlock_it.l_whence = SEEK_SET;
    unlock_it.l_start = 0;
    unlock_it.l_len = 0;
}

static pthread_mutex_t *mptr;

void my_lock_init2(const char *pathname)
{
    int fd;
    pthread_mutexattr_t mattr;

    fd = open("/dev/zero", O_RDWR, 0);
    mptr = (pthread_mutex_t *)mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, fd, 0);
    close(fd);

    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mptr, &mattr);
}

void my_lock_wait2()
{
    pthread_mutex_lock(mptr);
}

void my_lock_release2()
{
    pthread_mutex_unlock(mptr);
}

