# 用户, 文件操作与联机帮助: 编写who命令
---

### who 命令能做些什么
`who` 可以查看当前系统中有哪些用户在使用.
```shell
$ who
iccy     tty7         2022-07-02 00:09 (:0)
iccy     pts/0        2022-07-02 00:09 (:0)
iccy     pts/1        2022-07-02 00:09 (:0)
```

*ps: 使用关键词搜索信息*
```shell
man -k ${token}
```

通过阅读 `who` 和 `utmp` 的联机帮助, 以及头文件 `/usr/include/utmp.h` 可以得知 `who` 通过读取 `UTMP_FILE` 这个宏定义的文件(`/var/run/utmp`)来得到对应的信息.

其实就是反序列化该二进制文件的内容.
```c
struct utmp current_record; /* read info into here */
while (read(utmpfd, &current_record, reclen) == reclen) {
```

可见文件中的数据是与 `struct utmp` 一一对应的.
```c
/* The structure describing the status of a terminated process.  This
   type is used in `struct utmp' below.  */
struct exit_status
{
  short int e_termination;	/* Process termination status.  */
  short int e_exit;		/* Process exit status.  */
};


/* The structure describing an entry in the user accounting database.  */
struct utmp
{
  short int ut_type;		/* Type of login.  */
  pid_t ut_pid;			/* Process ID of login process.  */
  char ut_line[UT_LINESIZE]
    __attribute_nonstring__;	/* Devicename.  */
  char ut_id[4]
    __attribute_nonstring__;	/* Inittab ID.  */
  char ut_user[UT_NAMESIZE]
    __attribute_nonstring__;	/* Username.  */
  char ut_host[UT_HOSTSIZE]
    __attribute_nonstring__;	/* Hostname for remote login.  */
  struct exit_status ut_exit;	/* Exit status of a process marked
				   as DEAD_PROCESS.  */
/* The ut_session and ut_tv fields must be the same size when compiled
   32- and 64-bit.  This allows data files and shared memory to be
   shared between 32- and 64-bit applications.  */
#if __WORDSIZE_TIME64_COMPAT32
  int32_t ut_session;		/* Session ID, used for windowing.  */
  struct
  {
    int32_t tv_sec;		/* Seconds.  */
    int32_t tv_usec;		/* Microseconds.  */
  } ut_tv;			/* Time entry was made.  */
#else
  long int ut_session;		/* Session ID, used for windowing.  */
  struct timeval ut_tv;		/* Time entry was made.  */
#endif

  int32_t ut_addr_v6[4];	/* Internet address of remote host.  */
  char __glibc_reserved[20];		/* Reserved for future use.  */
};
```