#AIOCC安装手册


##软硬件环境
操作系统：Centos-7.2,安装操作系统的时候,划分磁盘分区的时候不要使用系统默认方案,需要自行划分分区,OSS和MDS要留出至少一个单独的分区给Lustre使用
Lustre版本：2.9

sh update_to_python3.5.2.sh
pip install tensorflow_gpu-1.6.0-cp35-cp35m-linux_x86_64.whl