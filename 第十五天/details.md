#MyOS第十五天#
###day26###
　　窗口提速，对有透明色和无透明色的图层刷新机制进行了优化。之前为了酷一点，我在窗口的标题栏部分使用了透明色，现在为了性能，只好吧透明色去掉了。<br><br>
　　在对命令行窗口进行优化完善，start命令正常运行，然而ncst命令却死活不对劲。开机直接ncst，不报错，也没有打开对应应用程序，先运行start命令，再ncst时，可以打开刚刚start打开的应用程序。不知道bug在哪里，明天再看吧。