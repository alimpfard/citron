#!/usr/bin/env bash

# set -v
# set -x

is_broken_test() {
    case "$1" in
        tests/test0013.ctr|\
        tests/test0041.ctr|\
        tests/test0098.ctr|\
        tests/test0099.ctr|\
        tests/test0100.ctr|\
        tests/test0119.ctr|\
        tests/test0120.ctr|\
        tests/test0140.ctr|\
        tests/test0142.ctr|\
        tests/test0143.ctr|\
        tests/test0144.ctr|\
        tests/test0145.ctr|\
        tests/test0148.ctr|\
        tests/test0149.ctr|\
        tests/test0152.ctr|\
        tests/test0153.ctr|\
        tests/test0155.ctr|\
        tests/test0157.ctr|\
        tests/test0158.ctr|\
        tests/test0162.ctr|\
        tests/test0163.ctr|\
        tests/test0164.ctr|\
        tests/test0165.ctr|\
        tests/test0167.ctr|\
        tests/test0168.ctr|\
        tests/test0169.ctr|\
        tests/test0173.ctr|\
        tests/test0174.ctr|\
        tests/test0190.ctr|\
        tests/test0193.ctr|\
        tests/test0195.ctr|\
        tests/test0206.ctr|\
        tests/test0229.ctr|\
        tests/test0232.ctr|\
        tests/test0268.ctr|\
        tests/test0270.ctr|\
        tests/test0292.ctr|\
        tests/test0293.ctr|\
        tests/test0294.ctr|\
        tests/test0299.ctr|\
        tests/test0303.ctr)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

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
    if is_broken_test "$i"; then
        echo "Skipping broken test $i"
        continue
    fi
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
	result=`echo "$useless_input" | timeout 15 ${pre_c}/ctr --vm --compact ${fitem}`
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
    if $crash; then
        echo " [$j]"
        j=$((j+1))
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
