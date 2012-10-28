#!/bin/sh

export LD_LIBRARY_PATH=.
cd $(pwd -P)
lua smartlan.lua 2> /dev/null &
