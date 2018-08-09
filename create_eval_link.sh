#!/bin/bash
set -x
printf "%s%s\n" '#!' "`which sh`" > run.sh
echo 'cwd=$(pwd)' >> run.sh
printf 'cd %s && ./ctr eval --assume-non-tty --without-signals --without-history --terminal-width 40 "%s"\n' "`pwd`" '$@' >> run.sh
echo 'cd $cwd' >> run.sh
chmod a+x run.sh
mv run.sh /data/data/com.termux/files/usr/bin/citron
