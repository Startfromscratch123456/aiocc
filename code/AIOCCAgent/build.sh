#!/usr/bin/env bash
#author dengshijun1992@gmail.com
#2017-07-19
# POSIX
# aiocc监控控制程序 - 编译安装
#############################################

# if the environment has been setup before clean it up
if [ $GOBIN ]; then
    PATH=$(echo $PATH | sed -e "s;\(^$GOBIN:\|:$GOBIN$\|:$GOBIN\(:\)\);\2;g")
fi
GO_INSTALL_PATH="/usr/local/"
mkdir -p ${GO_INSTALL_PATH}
export PATH=$PATH:${GO_INSTALL_PATH}/go/bin
BUILD_DIR=$PWD/build
ANGENT_NAME=aiocc

if [ ! -d ${GO_INSTALL_PATH}/go ]; then
    cp -rf ${BUILD_DIR}/go ${GO_INSTALL_PATH}
fi

export CTEST_OUTPUT_ON_FAILURE=1
export GOPATH=$BUILD_DIR/${ANGENT_NAME}
export LD_LIBRARY_PATH=$BUILD_DIR/${ANGENT_NAME}/lib
export DYLD_LIBRARY_PATH=$BUILD_DIR/${ANGENT_NAME}/lib
export GOBIN=$GOPATH/bin
export PATH=$GOBIN:$PATH

srcfile=main/aiocc.go
module=main/aiocc
go build -o ${module} ${srcfile}

code=$?
if [ $code -eq 0 ]; then
    echo "build successful..."
    exit 0
else
    echo "build failed..."
    exit $code
fi