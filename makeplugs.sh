plugs=("curl" "ffi" "gmp" "json" "sdl" "keyboardmods" "tcl")

echo "Building $plugs"

for plug in ${plugs[*]}
do
    echo "building $plug";
    cd "plugins/$plug";
    if [[ -e "./configure" ]]; then
      ./configure
    fi
    echo $(make && echo "Succ $plug" || echo "$plug Failed")
    cd ../..
done
