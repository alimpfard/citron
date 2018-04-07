plugs=("curl" "ffi" "gmp" "json" "sdl" "keyboardmods")

echo "Building $plugs"

for plug in ${plugs[*]}
do
    echo "building $plug";
    cd "plugins/$plug";
    if [[ -e "./configure" ]]; then
      ./configure
    fi
    echo $(make >/dev/null 2>&1 && echo "Succ $plug" || echo "$plug Failed")
    cd ../..
done
