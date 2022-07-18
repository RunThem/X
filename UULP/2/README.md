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
10+0 records in
10+0 records out
10485760 bytes (10 MB, 10 MiB) copied, 0.210118 s, 49.9 MB/s
++++++++++++++++++++++++++++++
buffer size 1 byte.

real    0m4.909s
user    0m2.060s
sys     0m2.848s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 4 byte.

real    0m1.232s
user    0m0.536s
sys     0m0.696s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 16 byte.

real    0m0.314s
user    0m0.112s
sys     0m0.203s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 128 byte.

real    0m0.042s
user    0m0.034s
sys     0m0.008s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 256 byte.

real    0m0.021s
user    0m0.007s
sys     0m0.014s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 512 byte.

real    0m0.011s
user    0m0.006s
sys     0m0.005s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 1024 byte.

real    0m0.007s
user    0m0.000s
sys     0m0.007s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 2048 byte.

real    0m0.004s
user    0m0.000s
sys     0m0.004s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 4096 byte.

real    0m0.003s
user    0m0.000s
sys     0m0.002s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 8192 byte.

real    0m0.002s
user    0m0.002s
sys     0m0.000s
++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++
buffer size 16384 byte.

real    0m0.001s
user    0m0.001s
sys     0m0.000s
++++++++++++++++++++++++++++++
```

上面是在不同的缓冲区大小后读取 `10MB` 的文件所需要的时间, 可以看到在 `4096byte` 内的缓冲区越大效率越好, 但超过该阈值效果就不大了.

系统调用的非常耗时的, 但暂时不太清楚是具体怎么调用的.

### Q

可以在 `who` 中使用缓冲吗

### A

当然可以了, 在原版的 `who` 中是每一个用户都使用一次系统调用, 可以一次性读取多个放入缓冲区内, 要的时候再从缓冲区中读取,
若缓冲区空了再次调用放入缓冲区中. 详情可以看 `utmplib.c`.

---

### 注销帐号的过程在做什么?

按书上说的是对 `/var/run/utmp` 这个文件修改内容即可, 但看其他的书上似乎不是这样的, 暂时先不管它.

大致的过程如下:

1. 打开文件 `utmp`
2. 从 `utmp` 找到你所在的终端的登录记录
3. 对当前记录做出修改
4. 关闭文件

第三步的重点就是将 `ut_type` 字段的值从 `USER_PROCESS` 改成 `DEAD_PROCESS`, 将 `ut_time`
字段的值改为注销时间, 然后将文件回写进文件中.

但是这个地方有个文件, 我们是读取出了一条记录了, 此时指向文件当前位置的指针走到了该记录的结尾, 我们再使用 `write()`
写入的记录是写到了下一条记录中

```text
				我们的记录所在
					   |	
			+------+------+------+-----+
			| utmp | utmp | utmp | utmp|
			+------+------+------+-----+
				          ^
						  |
					指向文件当前文件的指针
```

我们需要调整这个指针该怎么办呢, 是有一个系统调用可以去调整的 `lseek()`.

## 题一

`w` 命令显示有谁登录以及他们在做什么. 大致如下:

```shell
$ w
 21:23:09 up 1 day, 56 min,  4 users,  load average: 0.50, 0.83, 0.78
USER     TTY      FROM             LOGIN@   IDLE   JCPU   PCPU WHAT
iccy     tty7     :0               Sun20   24:56m 10:09   0.09s /usr/bin/startplasma-x11
iccy     pts/0    :0               Sun20   24:55m  0.00s 11.49s /usr/bin/kded5
iccy     pts/1    :0               Sun20    5.00s 10:57   0.00s w
```

标题中显示的信息有 `w` 命令执行的时间, 系统运行了多长时间, 当gj前有几个用户登录了, 以及过去1, 5, 15分钟的系统负载平均值.

下面为每个用户显示以下条目: 登录名, tty名称, 远程主机, 登录时间, 空闲时间, JCPU, PCPU以及他们当前进程的命令行

前面一部分和 `who` 是一样的, 至于后面的信息是通过 `utmp` 中的 `ut_pid` 得到当前用户运行的进程ID, 在根据该ID从 `/proc`
中得到详细的信息. 标题中的信息应该也有一部分是从 `proc` 中得到的.

## 题二

这个问题的答案大致在 `man wtmp` 中, `wtmp` 是与 `utmp` 差不多的文件, 区别是 `wtmp` 记录着系统启动以来的所有 `utmp`
更改, 每次更改 `utmp` 都会同步的追加到 `wtmp` 中.

在正常状态下, `wtmp` 中的登录与注销消息数量是相等的, 就如同 `()` 一样, 是配对的, 若在非正常状态下, 那必然有多余的登录消息,
那就会为已这多余的信息重建可用终端记录.(猜想的)