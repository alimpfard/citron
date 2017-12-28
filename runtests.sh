#!/bin/sh

# set -v
# set -x

echo "Where is your libffi? tell me, please!"
ffi_path="/usr/local/lib";
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$ffi_path";
OS=`uname -s`
# make clean;
# make;
failing=()
j=1
useless_input="test
";
for i in $(find tests -name 'test*.ctr'); do
	fitem=$i
	echo -n "$fitem interpret";
	fexpect="${i%%.ctr}.exp"
	result=`echo "$useless_input" | ./ctr ${fitem}`
	expected=`cat $fexpect`
	if [ "$result" = "$expected" ]; then
		echo "[$j]"
		j=$((j+1))
	else
		echo "FAIL."
		echo "EXPECTED:"
		echo $expected
		echo ""
		echo "BUT GOT:"
		echo $result
		failing+=("$fitem");
	fi
	headline=$(head -n 1 $fitem)
done
echo ""
echo Tests failing: $failing
exit 0
