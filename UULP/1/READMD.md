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

## 动手实践
`more` -- 分页显示文件的内容.

`more` 有三种用法:
> $ more filename        # 显示文件filename的内容
> $ command | more   # 将command命令的输出分页显示
> $ more < filename    # 从标准输入获取分页显示的内容, 而此时more的标准输入被重定向到文件filename

`more` 是如何实现的:
```txt
+------> show 24 line from input
|  +---> print [more?] message
|  |     Input Enter, SPACE, or q
|  +---  if Enter, advance one line
+------  if SPACE
         if q --> exit
```