#!/bin/bash

export QT_SHORT_VERSION=5.7
export QT_LONG_VERSION=5.7.0
export QT_INSTALLER_ROOT=qt-opensource-mac-x64-clang-${QT_LONG_VERSION}
export QT_INSTALLER_FILENAME=${QT_INSTALLER_ROOT}.dmg

export QT_PATH=$HOME/qt
export QT_MACOS=$QT_PATH/$QT_SHORT_VERSION/clang_64
export PATH=$PATH:$QT_MACOS/bin

