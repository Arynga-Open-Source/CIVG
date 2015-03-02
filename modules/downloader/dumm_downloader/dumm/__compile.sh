#!/bin/sh +e

## author: bj@open-rnd.pl
## simple script for compiling this module for Tizen platform

GITCREATE=no

#rm -fr .git

if [ ! -d .git ] 
then
    echo Creating empty git repo ...
    git init
    GITCREATE=yes
fi

if [ -z  "$1"  ] ; then
    gbs -v  build   -A i586 --include-all 
else
    gbs -v  build   -A i586 --include-all -R $1
fi

RES=$?

if [ "$GITCREATE" = "yes" ]
then
    rm -fr .git
fi

exit $RES