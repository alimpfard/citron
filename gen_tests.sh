echo "" > failed_tests
dummy_input="test
"
fnames=()
echo $# -- $!
if [[ $# -ne 0 ]]; then
  fnames=($@)
else
  fnames=$(find tests -maxdepth 1 -name 'test*.ctr' | sort)
fi
for i in $fnames; do
    dest=${i%%.ctr}.exp
    echo "Generating results for $i > $dest"
    res=$(echo $dummy_input | timeout 5 ./ctr $i 2>&1)
    err=$?
    if [ $err -ne 0 ] && [ $err -ne 1 ]; then
        echo "Test case $res failed to run (status $err)"
        continue
    fi
    echo "$res" > $dest
    echo "Finished $i with status $err"
    echo "$i : $err" >> failed_tests
done
cat failed_tests
