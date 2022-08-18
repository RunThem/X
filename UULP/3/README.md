# 目录与文件属性: 编写ls
---

### ls 命令能做什么

`ls` 可以列出文件名与文件的属性, 参数众多, 是 `Unix` 系统中最常用的命令之一.

```shell
$ ls
2_6.c  ac.c           cat.c  head.c  logout.c  README.md  utmplib.c  who.c
2_9.c  buffer_test.c  cp.c   last.c  Makefile  tail.c     whoami.c
```

### 文件树

在 `Unix` 中的磁盘上, 文件与目录被组成一棵树, 每一个节点都是目录或文件,
每一个文件都是位于某个目录中, 在逻辑上没有驱动器或卷.
在物理上的多个驱动器或分区上的目录都是通过文件树无缝地连接在一起,
这点与 `Windows` 不同.

### ls 是如何工作的

大致逻辑如下:

```txt
    open directory
+-> read entry            - end of dir? -+
|__ display file info                    |
    close directory       <--------------+
```

#### 什么是目录

目录一种特殊的文件, 保存着文件与目录的名字, 与 `utmp` 是类似的, 目录与普通文件不同的是,
目录永远不为空, 每个目录至少包含两个特殊的项 -- `.` 与 `..`, `.`
表示当前目录, `..` 表示上一级目录, 其中 `/..` 是的特殊的, 与 `/.` 等同.

### 如何读取目录文件

既然目录只是特殊的文件, 那 `open(), read(), close()` 等系统调用也是可以打开目录文件的,
只是在 `Unix` 系统中是存在着不同的文件系统的, 如 `VFAT, EXT3, EXT4, Btrfs` 等,
这些文件系统的目录是有着各自的结构, 使用 `open()` 等系统调用是不明智的(
其实文件也是有不同的结构的, 只是 `open()` 等系统调用屏蔽了这些不同),
那么有没有屏蔽掉不同文件系统的目录结构不同吗?

我们可以找一下.

```shell
$ man -k direct | grep read
```

可以看到 `readdir` 等信息.

通过 `man 3 readdir` 我们可以得到针对目录文件的也有着库函数调用
-- `opendir(), readdir(), closedir()` 等

### 目录的结构

目录是文件的列表, 是记录的序列, 每条记录对应一个文件或子目录, `readdir()`
返回一个指向目录的当前记录的指针, 类型为 `struct dirent`, 记录在 `dirent.h` 中.

```c
struct dirent {
  ino_t          d_ino;       /* Inode number */
  off_t          d_off;       /* Not an offset; see below */
  unsigned short d_reclen;    /* Length of this record */
  unsigned char  d_type;      /* Type of file; not supported
                                 by all filesystem types */
  char           d_name[256]; /* Null-terminated filename */
};
```

ps: 在我的系统(`Debian`)中, `d_name` 的定义与书上的不同.

### 编写 ls

`ls` 的伪代码如下(详情看代码):

```txt
main()
  opendir
  while (readdir)
    print d_name
  closedir
```

### ls -l

标准的 `ls` 输出如下

```shell
total 32
-rw-r--r-- 1 iccy iccy    67 Aug 18 21:27 Makefile
-rw-r--r-- 1 iccy iccy  2791 Aug 18 22:36 README.md
-rwxr-xr-x 1 iccy iccy 16824 Aug 18 22:07 ls*
-rw-r--r-- 1 iccy iccy  3420 Aug 18 22:07 ls.c
```

共七栏:

* 模式(mode): 第一字符表示文件类型, '-' 表示普通文件, 'd' 表述目录,
  其余九个字符表述文件访问权限
* 链接数(links): 指该文件被引用次数
* 文件所有者(owner): 文件所有者的用户名
* 组(group): 文件所有者所在的组
* 大小(group): 文件大小, 目录的大小是相同的, 均为块大小; 文件则是实际的字节数
* 最后修改时间(last-modified): 文件的最后一次修改的时间
* 文件名(name): 文件名

### 如何得到文件的详细信息呢

`man -k file | grep status` 可以看到 `stat()` 这个函数,
详情可以看看 `man 2 stat`

可以看到文件详细信息是存放在 `struct stat` 这个结构题中的, 定义在 `sys/stat.h` 中

```c
struct stat {
  dev_t     st_dev;         /* ID of device containing file */
  ino_t     st_ino;         /* Inode number */
  mode_t    st_mode;        /* File type and mode */
  nlink_t   st_nlink;       /* Number of hard links */
  uid_t     st_uid;         /* User ID of owner */
  gid_t     st_gid;         /* Group ID of owner */
  dev_t     st_rdev;        /* Device ID (if special file) */
  off_t     st_size;        /* Total size, in bytes */
  blksize_t st_blksize;     /* Block size for filesystem I/O */
  blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */

  /* Since Linux 2.6, the kernel supports nanosecond
     precision for the following timestamp fields.
     For the details before Linux 2.6, see NOTES. */

  struct timespec st_atim;  /* Time of last access */
  struct timespec st_mtim;  /* Time of last modification */
  struct timespec st_ctim;  /* Time of last status change */

#define st_atime st_atim.tv_sec      /* Backward compatibility */
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
};

```

其他的字段好理解, `st_mode` 就比较特殊了, 该字段为16位的二进制数,
文件类型与权限都被编码在这个字段中

```txt
____ ___ ___ ___ ___

# 该16个位分为五个段
# 第一个段占4位, 用作文件类型, 最多标识16种类型, 目前使用了7个
# 第二个段占3位, 是文件的特殊属性, 分别是 `set-user-ID`, `set-group-ID`, `sticky` 这三个属性,
# 最后9位是许可权限, 分3组, 对应着3种用户, 分别是文件所有者, 同组用户和其他用户(不是用组的人). 每组分别是读, 写和执行的权限.
```

#### 如何从 st_mode 中读出指定数据呢

我们可以使用掩码来得到数据, 在 `sys/stat.h` 中有以下定义:

```c
# define S_IFMT		__S_IFMT
# define S_IFDIR	__S_IFDIR
# define S_IFCHR	__S_IFCHR
# define S_IFBLK	__S_IFBLK
# define S_IFREG	__S_IFREG
# ifdef __S_IFIFO
#  define S_IFIFO	__S_IFIFO
# endif
# ifdef __S_IFLNK
#  define S_IFLNK	__S_IFLNK
# endif
# if (defined __USE_MISC || defined __USE_XOPEN_EXTENDED) \
     && defined __S_IFSOCK
#  define S_IFSOCK	__S_IFSOCK
# endif
#endif
```

`S_IFMT` 是一个掩码, 值为 `0170000`, 可以过滤出前四位表示的文件类型, `S_IFREG`
表示普通文件, `S_IFDIR` 表示目录文件, 可以使用 `info.st_mode & S_IFMT` 来得到文件的类型,
不过在头文件中有这样的宏定义了.

```c
#define	__S_ISTYPE(mode, mask)	(((mode) & __S_IFMT) == (mask))

#define	S_ISDIR(mode)	 __S_ISTYPE((mode), __S_IFDIR)
#define	S_ISCHR(mode)	 __S_ISTYPE((mode), __S_IFCHR)
#define	S_ISBLK(mode)	 __S_ISTYPE((mode), __S_IFBLK)
#define	S_ISREG(mode)	 __S_ISTYPE((mode), __S_IFREG)
#ifdef __S_IFIFO
# define S_ISFIFO(mode)	 __S_ISTYPE((mode), __S_IFIFO)
#endif
#ifdef __S_IFLNK
# define S_ISLNK(mode)	 __S_ISTYPE((mode), __S_IFLNK)
#endif
```

只需要使用即可 `S_ISDIR(info.st_mode)`

至于将 `st_mode` 编码成字符串与将用户/组ID转换成字符串在此处不再多多说, 详情看代码.