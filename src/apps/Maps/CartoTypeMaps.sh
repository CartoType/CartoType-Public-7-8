#!/bin/sh

# This shell script sets the shared library path and runs CartoTypeMaps: see http://doc.qt.io/qt-5/linux-deployment.html

appname=`basename $0 | sed s,\.sh$,,`

dirname=`dirname $0`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi

# For the moment (since upgrading to Qt 15.1 on Ubuntu) setting the library path is disabled, because it prevented the OpenGL context from being created,
# giving the error "QXcbIntegration: Cannot create platform OpenGL context, neither GLX nor EGL are enabled"
# LD_LIBRARY_PATH=$dirname
# export LD_LIBRARY_PATH

$dirname/$appname "$@"
