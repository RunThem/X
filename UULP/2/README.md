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

通过阅读 `who` 和 `utmp` 的联机帮助, 以及头文件 `/usr/include/utmp.h` 可以得知 `who`
通过读取 `UTMP_FILE` 这个宏定义的文件(`/var/run/utmp`)来得到对应的信息.

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

#### Q

当前版本有点小问题, 没有消除空白的记录和正确的显示时间.

```
(~/D/X/U/2) ./who 
reboot   ~        1657435942 (5.16.0-1-amd64)
runlevel ~        1657435995 (5.16.0-1-amd64)
iccy     tty7     1657436023 (:0)
iccy     pts/0    1657436024 (:0)
iccy     pts/1    1657436025 (:0)
         pts/2    1657436155 (:0)
```

可以看到最后一行是无效的, 另外所有的时间都有问题.

#### A

消除无效记录好办, `struct utmp` 中有一个字段 `ut_type` 即使用来区分不同的用户, 在 `utmp.h` 中有着以下宏是该字段的有效值.

```c
/* Values for the `ut_type' field of a `struct utmp'.  */
#define EMPTY		0	/* No valid user accounting information.  */

#define RUN_LVL		1	/* The system's runlevel.  */
#define BOOT_TIME	2	/* Time of system boot.  */
#define NEW_TIME	3	/* Time after system clock changed.  */
#define OLD_TIME	4	/* Time when system clock changed.  */

#define INIT_PROCESS	5	/* Process spawned by the init process.  */
#define LOGIN_PROCESS	6	/* Session leader of a logged in user.  */
#define USER_PROCESS	7	/* Normal process.  */
#define DEAD_PROCESS	8	/* Terminated process.  */

#define ACCOUNTING	9
```

其中 `USER_PROCESS` 表示这是已经登录的用户, 那这加个if即可.

至于转换一下时间, 和书上的不太一样, 在书上的版本中, `ut_tv` 的类型是 `time_t`, 使用 `ctime()` 来格式化为可读的形式,
但在我的 `Debian 11` 中其类型为 `struct timeval`, 格式化有点复杂, 主要使用 `strftime()`.

```c
/* https://stackoverflow.com/questions/2408976/struct-timeval-to-printable-format */

struct timeval tv;
time_t nowtime;
struct tm *nowtm;
char tmbuf[64], buf[64];

gettimeofday(&tv, NULL);
nowtime = tv.tv_sec;
nowtm = localtime(&nowtime);
strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, tv.tv_usec);
```

`who` 命令就到此结束了, 来看看 `cp` 命令怎么写?

### cp 命令可以做什么

`cp` 命令可以复制文件, 如果目标文件不存在, 则创建该文件, 如果文件存在则覆盖掉.

```shell
cp ${source-file} ${target-file}
```

`cp` 命令的处理流程如下:

```txt
   open source file for reading
   open copy file for writing
+->read from source to buffer -- eof?   -+
+- write from buffer to copy             |
                                         |
   close source file       <-------------+
   close copy file 
```

*ps: 可以使用linux自带的 `cmp` 文件对比工具测试 `cp` 命令是否有效*

```shell
cmp ${source-file} ${target-file}
```

---

### Q

在 `who` 和 `cp` 命令中, 都有一个共同的操作 -- 从文件中读取内容, 并且每次读取的长度固定, 那这个长度是否对性能有影响吗,
改大点会怎么样呢?

### A

确实影响很大, 我们可以做个实验.

```shell
$ make buffer_test
cc buffer_test.c -o buffer_test
dd if=/dev/random of=test.file bs=1M count=10
记录了10+0 的读入
记录了10+0 的写出
10485760字节（10 MB，10 MiB）已复制，0.213696 s，49.1 MB/s
++++++++++++++++++++++++++++++
buffer size 1 byte.

real    0m4.910s
user    0m2.157s
sys     0m2.753s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 4 byte.

real    0m1.214s
user    0m0.507s
sys     0m0.707s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 16 byte.

real    0m0.311s
user    0m0.156s
sys     0m0.155s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 128 byte.

real    0m0.042s
user    0m0.017s
sys     0m0.025s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 256 byte.
 
real    0m0.022s
user    0m0.017s
sys     0m0.004s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 512 byte.
 
real    0m0.012s
user    0m0.012s
sys     0m0.000s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 1024 byte.
 
real    0m0.007s
user    0m0.000s
sys     0m0.007s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 2048 byte.
 
real    0m0.004s
user    0m0.000s
sys     0m0.004s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 4096 byte.
 
real    0m0.003s
user    0m0.000s
sys     0m0.003s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 8192 byte.
 
real    0m0.002s
user    0m0.002s
sys     0m0.000s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 16384 byte.
 
real    0m0.002s
user    0m0.000s
sys     0m0.002s
++++++++++++++++++++++++++++++
```

上面是在不同的缓冲区大小后读取 `10MB` 的文件所需要的时间, 可以看到在 `4096byte` 内的缓冲区越大效率越好, 但超过该阈值效果就不大了.

系统调用的非常耗时的, 但暂时不太清楚是具体怎么调用的.

### Q

可以在 `who` 中使用缓冲吗

### A

当然可以了, 在原版的 `who` 中是每一个用户都使用一次系统调用, 可以一次性读取多个放入缓冲区内, 要的时候再从缓冲区中读取,
若缓冲区空了再次调用放入缓冲区中.