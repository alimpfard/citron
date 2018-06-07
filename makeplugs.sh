plugs=("curl" "ffi" "gmp" "json" "sdl" "keyboardmods" "tcl")

has_citron=$(echo "#include <Citron/citron.h>" | gcc -E -)
has_citron=$?
makefvar=""

if [[ $has_citron -eq 0 ]]; then
    makefvar="-Dexisting=1 -DCTR_STD_EXTENSION_PATH=\"$(ctr --ext)\""
fi

echo "built Citron: " $makefvar

for plug in ${plugs[*]}
do
    echo "building $plug";
    cd "plugins/$plug";
    if [[ -e "./configure" ]]; then
      ./configure
    fi
    echo $(sudo make "EXTRAS=$makefvar" >/dev/null 2>&1 && echo "Succ $plug" || echo "$plug Failed")
    cd ../..
done
