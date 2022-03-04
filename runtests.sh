#!/bin/sh

# set -v
# set -x

dir=$(pwd)
pre_c='build'
if [ "x$1" != "x" ]; then
    pre_c=$1
fi
failing=()
j=1
useless_input="test
";
cd $dir
for i in $(find tests -maxdepth 1 -name 'test*.ctr' | sort --version-sort); do
	fexpect="${i%%.ctr}.exp"
    fcrash="${i%%.ctr}.crash"
    crash=false
    if [ ! -f $fexpect ]; then
        if [ ! -f $fcrash ]; then
            echo "No expect file, skipping $i"
            continue
        fi
        crash=true
    fi
    fitem=$i
	echo -n "$fitem interpret";
	result=`echo "$useless_input" | timeout 15 ${pre_c}/ctr --compact ${fitem}`
    rv=$?
    if [ $rv -ne 0 ] && [ $rv -ne 1 ]; then
        if ! $crash; then
            echo " [Failed with result $rv]"
            failing+=($fitem)
            continue
        fi
        result=''
    elif $crash; then
        echo " [Failed, expected crash]"
        failing+=($fitem)
        continue
    fi
	expected=`cat $fexpect`
	if [ "$result" = "$expected" ]; then
		echo " [$j]"
		j=$((j+1))
	else
		echo "FAIL."
		echo "EXPECTED:"
		echo $expected
		echo ""
		echo "BUT GOT:"
		echo $result
        failing+=("$fitem");
	    # sleep 0.5 && echo ""
    fi
	headline=$(head -n 1 $fitem)
    # sleep 0.2 && clear
done
echo ""
if [ ${#failing[@]} -gt 10 ]; then
    echo Tests failing: ${failing[@]}
    exit 1
fi
exit 0
