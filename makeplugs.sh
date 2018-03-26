plugs=("curl" "ffi" "gmp" "json" "sdl")

echo "Building $plugs"

for plug in ${plugs[*]}
do
    echo "building $plug";
    cd "plugins/$plug";
    echo $(make >/dev/null 2>&1 && echo "Succ $plug" || echo "$plug Failed")
    cd ../..
done
