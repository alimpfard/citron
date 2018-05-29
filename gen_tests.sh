echo "" > failed_tests
dummy_input="blah blah"
fnames=()
echo $# -- $!
if [[ $# -ne 0 ]]; then
  fnames=($@)
else
  fnames=$(find tests -maxdepth 1 -name 'test*.ctr' | sort)
fi
for i in $fnames; do
    dest=$(echo $i | sed -r 's/(.*)\.ctr/\1.exp/')
    echo "Generating results for $i > $dest"
    res=$(echo $dummy_input | timeout 5 ./ctr $i 2>&1)
    err=$?
    echo "$res" > $dest
    echo "Finished $i with status $err"
    echo "$i : $err" >> failed_tests
done
cat failed_tests
