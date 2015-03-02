#!/bin/bash

# tizen helper script part, for more details see:
# https://gitorious.org/tizen/tizen-helper/
# @author: Philippe Coval <mailto:philippe.coval@eurogiciel.com>
# @description: manage git submodules with git-build-package-rpm

git submodule status --recursive | awk '{ print $2 }' | while read dir  ; do
    name=$(basename "$dir" )
    git submodule update --init --recursive
    tar cjvf "./packaging/${name}.tar.bz2" "${dir}"
done

gbs build -A i586 --include-all
cp ~/GBS-ROOT/local/repos/tizen/i586/RPMS/civg-2.0.0*.rpm .
