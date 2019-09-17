#!/bin/bash

if [[ ! -d $1 && ! -f $1 ]]
then
    exit 1
fi

SOURCE=$1

function copyTo() {
    DEST=$1

    if [[ ! -d $DEST ]]
    then
        mkdir $DEST
    fi

    rm -rf $DEST/$SOURCE && cp -r $SOURCE $DEST
    ln adjust.cnf $DEST
}

copyTo bin
copyTo bin/debug
copyTo bin/release
copyTo bin/relwithdebinfo
copyTo bin/minsizerel
