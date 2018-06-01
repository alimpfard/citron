#!/bin/sh

# set -v
# set -x

OS=`uname -s`
failing=()
j=1
useless_input="test
";
for i in $(find tests -maxdepth 1 -name 'test*.ctr'); do
	fitem=$i
	echo -n "$fitem interpret";
	fexpect="${i%%.ctr}.exp"
	result=`echo "$useless_input" | timeout 15 ./ctr ${fitem}`
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
