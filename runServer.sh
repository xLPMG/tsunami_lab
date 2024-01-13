#!/bin/bash

# wait for 2s to make sure the server stopped
sleep 2

# check if first argument is not empty
# the second argument are the compiler options which will be appended in any case
echo Compiling with:
if [ -n "$1" ]
then
#use first argument as environmental variable
echo export $1; scons $2
export $1; scons $2
else
echo scons $2
scons $2
fi

# restart the server
echo STARTING SERVER
./build/tsunami_lab