#!/bin/bash

CS_PATH=$1
CS_PIPE=$2
CS_NODE=4c6d84a0-2285-4653-0004-000000000001

TEMP_DIR=/mnt/carsync/temp
DEST_DIR=/home

set -e

function run {
    log "Running script..."
    rm -fr $TEMP_DIR
}
function log
{
    echo $CS_NODE:$1 > $CS_PIPE
}

if [[ ! -p $CS_PIPE ]]
then
    exit 1
fi

if [ -e "$CS_PATH/partial_$CS_NAME" ]
then
    bash $CS_PATH/partial_$CS_NAME $CS_PATH $CS_PIPE
else
    run
fi


exit 0
