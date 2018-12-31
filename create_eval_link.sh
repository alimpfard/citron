#!/bin/bash
set -x
printf "%s%s\n" '#!' "`which sh`" > run.sh
echo 'cwd=$(pwd)' >> run.sh
printf 'cd %s && LD_LIBRARY_PATH="$LD_LIBRARY_PATH:." ./ctr eval --without-history "%s"\n' "`pwd`" '$@' >> run.sh
echo 'cd $cwd' >> run.sh
chmod a+x run.sh
# ;P craaaazy
mv run.sh $(dirname `which sh`)/citron 
