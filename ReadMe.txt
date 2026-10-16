USB-UART-BLE透传工程
USB使用串口助手
BLE在上位机可以用notify形式接到数据







命令行格式：
设置波特率：	tcc set baudrate <number>
设置数据位：	tcc set databits 5/6/7/8
设置校验位：	tcc set parity N/E/O/M/S
设置停止位：	tcc set stopbits 1/2
设置工作模式：	tcc set sysmode 1/2/3
设置蓝牙模式：	tcc set blemode 0/1
以上设置支持读取：	tcc get <item>
以上设置掉电不保存，若要保存到闪存，使用：
保存全部设置：	tcc save
退出命令行模式：	tcc exit
命令末尾使用 \n 和/或 \r 均可


































