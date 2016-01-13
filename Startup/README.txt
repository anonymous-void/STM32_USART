这个文件夹包含的是ARM核心启动文件。对于不同类型芯片有着不同的启动文件。
ld、md、hd分别代表低密度、中密度和高密度芯片
stm32f103RBT6 选择 startup_stm32f10x_md.s这个文件即可，需要在MDK工程中add file，并且加入到搜索路径中。