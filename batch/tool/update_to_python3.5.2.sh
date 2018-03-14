 #!/bin/bash
# POSIX
#
#description:    update python to version 3.5.2
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2016-12-29
#
#initialization
cd "$( dirname "${BASH_SOURCE[0]}" )" #get  a Bash script tell what directory it's stored in
if [ ! -f ../ctrl/__init.sh ]; then
        echo "MULTEXU Error:multexu initialization failure:cannot find the file __init.sh... "
    exit 1
else
    source ../ctrl/__init.sh
fi

source "${MULTEXU_BATCH_CRTL_DIR}/multexu_lib.sh"
clear_execute_statu_signal

print_message "MULTEXU_INFO" "Now start to update python..."
cd ${MULTEXU_SOURCE_TOOL_DIR}
print_message "MULTEXU_INFO" "Entering directory ${MULTEXU_SOURCE_TOOL_DIR}..."
yum -y install gcc 
wait
yum -y groupinstall "Development tools"
wait
yum -y wget install zlib-devel 
wait
yum -y install bzip2-devel 
wait
yum -y install openssl
wait
yum -y install openssl-devel
wait
yum -y install ncurses-devel 
wait
yum -y install readline-devel 
wait
yum -y install tk-devel 
wait
yum -y install gdbm-devel 
wait
yum -y install db4-devel 
wait
yum -y install libpcap-devel
wait
yum -y install  xz-devel 
wait
yum -y install sqlite 
wait
yum -y install sqlite-devel
wait
xz -d Python-3.5.2.tar.xz 
tar -xvf Python-3.5.2.tar 
mkdir /usr/local/python3.5.2
cd Python-3.5.2/
./configure --prefix=/usr/local/python3.5.2
wait

make
wait
make install
wait

echo yes | mv /usr/bin/python /usr/bin/python.bak
ln -s /usr/local/python3.5.2/bin/python3.5 /usr/bin/python
echo yes | mv /usr/local/bin/python /usr/local/bin/python.bak
ln -s /usr/bin/python /usr/local/bin/python 
#解决python升级yum第一行python版本引入问题
function repalcepythonhead()
{
    local count=`cat $1 | grep "python2.7" | wc -l`
    if [ $count -eq 0 ]; then
        sed -i "s/\/usr\/bin\/python/\/usr\/bin\/python2.7/g" $1    
    fi
}


repalcepythonhead "/usr/bin/yum"
repalcepythonhead "/usr/bin/yum-buildde"
repalcepythonhead "/usr/bin/yum-config-manager"
repalcepythonhead "/usr/bin/yum-debug-dump"
repalcepythonhead "/usr/bin/yum-debug-restore"
repalcepythonhead "/usr/bin/yumdownloader"
repalcepythonhead "/usr/bin/yum-groups-manager"
repalcepythonhead "/usr/libexec/urlgrabber-ext-down"

cd ${MULTEXU_SOURCE_TOOL_DIR}
tar -zxvf setuptools-19.6.tar.gz
cd setuptools-19.6
python setup.py build
python setup.py install

cd ${MULTEXU_SOURCE_TOOL_DIR}
tar -zxvf pip-8.0.2.tar.gz
cd pip-8.0.2
python setup.py build
python setup.py install

cd /usr/local/python3.5.2/bin/
ln -s /usr/local/python3.5.2/bin/pip3 /usr/bin/pip

#升级
pip install --upgrade pip
export PYTHONPATH=$PYTHONPATH:/usr/local/lib/python3.5/site-packages/

send_execute_statu_signal "${MULTEXU_STATUS_EXECUTE}"
print_message "MULTEXU_INFO" "Please check /usr/bin/yum manually..."
print_message "MULTEXU_INFO" "Leaving directory ${MULTEXU_SOURCE_TOOL_DIR}..."
print_message "MULTEXU_INFO" "finished to update python..."
