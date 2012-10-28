#!/bin/sh

export LD_LIBRARY_PATH=.
cd $(dirname $(readlink $0))
lua smartlan.lua 2> /dev/null &
