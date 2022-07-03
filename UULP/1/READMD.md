# Unix 系统编程概述

---

## 什么是系统编程?
计算机用操作系统来管理所有的资源, 并将不同的设备和不同的程序连接起来.
操作系统也是程序, 但她是个特殊的程序, 它能将普通程序与其他的程序或设备连接起来.

### 系统资源
1. 处理器(Processor)
2. 输入输出(I/O)
3. 进程管理(Process Management)
4. 内存(Memory)
5. 设备(Device)
6. 计时器(Timers)
7. 进程间通信(Interprocess Communication)
8. 网络(Networking)

## 动手实践 -- more
`more` -- 分页显示文件的内容.

`more` 有三种用法:
> $ more filename        # 显示文件filename的内容
> $ command | more   # 将command命令的输出分页显示
> $ more < filename    # 从标准输入获取分页显示的内容, 而此时more的标准输入被重定向到文件filename

`more` 工作的大致流程如下:
```txt
+------> show 24 line from input
|  +---> print [more?] message
|  |     Input Enter, SPACE, or q
|  +---  if Enter, advance one line
+------  if SPACE
         if q --> exit
```

0.1 版很简单, 可以正常的工作了.

#### Q
如果是使用管道来重定向标准输入的话, 该程序会在打印24行之后继续打印, 不会停下, 原因是 `getchar()` 是从标准输入读取数据的, 但标准输入已经被重定向到其他地方, 于是该程序使用同一个数据流中读取数据和用户的输入, 直接从数据中读取了"用户操作".
```bash
ls /bin | ./more
```

#### A
问题的原因就是上面的最后一句话, 那就反其道而行之, 直接从键盘中获取用户的输入即可解决问题了.

`/dev/tty` 是键盘与显示器的设备描述文件, 从这里获取用户的输入即可解决问题

#### Q
真正的 `more` 是不需要输入后按回车的, 也不会回显用户的输入的.

#### A
这个地方犯了点错, 我以为只要设置 `tty` 无缓冲即可, 可是没用. 其实是 `FILE` 有两层缓冲, 一层是 `libc` 的, 一层是 `kernel` 的, 我只关了 `libc` 那层的, 自然是无效的. `setvbuf()` 是控制 `libc` 的 `I/O` 缓冲的. `ioctl()` 才是控制 `kernel`的 `I/O` 的具体配置.
```c
#include <stdio_ext.h>
printf("FILE* buffer size is %zu\n", __fbufsize(fp)) /* size_t __fbufsize(FILE*) 用来查看文件指针的缓冲区大小 */

if (setvbuf(fp_tty, NULL, _IONBF, 0)) {
	exit(1);
}
```

`/dev/tty` 又是个终端, `setvbuf()` 似乎是针对终端无效的, 那正确的方式是使用 `termios.h(对ioctl()的封装)` 里面的终端函数.
```c
struct termios ctrl;
tcgetattr(fileno(fp_tty), &ctrl); /* int fileno(FILE*)获取文件指针中的文件描述符, 不要使用fp_tty->xxx来得到这个描述符, 因为FILE*中的字段都不是对外的, 不同的libc中的实现是不同的, 至少glibc和musl不同 */
ctrl.c_lflag &= ~ICANON; /* turning off canonical mode makes input unbuffered */
tcsetattr(fileno(fp_tty), TCSANOW, &ctrl);
```

至于说按键回显的问题也是一样的, 去掉终端里的 `ECHO` 属性即可.
```c
ctrl.c_lflag &= ~(ICANON | ECHO); /* turning off canonical mode makes input unbuffered, also make the input echoless */
```

#### Q
在 `more` 中, 底下一行的不会向上移动的,  但现在我们的这个版本还不行啊

#### A
我们在打印 "more?" 时没有换行, 只需将光标设置到行首, 打印出几个空格覆盖掉 "more?" 再次使光标回到行首即可, `\r` 就是起这个作用的.
```c
fputs("\r       \r", stdout);
```

#### Q
我们固定一页是24行, 这可不是什么好事.

#### A
我们可以使用 `iotcl()` 获取终端的大小. (没有考虑虚拟终端更改大小的情况, 只在程序启动时获取一次).
```c
struct winsize ws;
ioctl(fileno(fp_tty), TIOCGWINSZ, &ws);
page_len = ws.ws_row;
```

## other Q
得到文件中已显示的百分比, 但这个需要知道文件的行数而不是文件大小, 有点性能问题啊, 不可能把一个大文件全读了去得到行数吧. 还有书上说终端类型定为 `vt100` 是什么意思? 反白显示文字?