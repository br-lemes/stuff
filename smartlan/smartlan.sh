#!/bin/sh

export LD_LIBRARY_PATH=.
cd $(dirname $(readlink -f $0))
lua5.1 smartlan.lua 2> /dev/null &
