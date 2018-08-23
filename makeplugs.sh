plugs=("curl" "gmp" "json" "sdl" "keyboardmods" "tcl")

has_citron=$(echo "#include <Citron/citron.h>" | gcc -E -)
has_citron=$?
makefvar=""
here=`pwd`

if [[ $has_citron -eq 0 ]]; then
    #makefvar="-Dexisting=1 -DCTR_STD_EXTENSION_PATH=\"$(ctr --ext)\""
    echo "Had Citron"
fi

echo "built Citron: " $makefvar

for plug in ${plugs[*]}
do
    echo "building $plug";
    cd "$here/plugins/$plug";
    if [[ -e "./configure" ]]; then
      ./configure
    fi
    echo $(sudo make "EXTRAS=$makefvar" && echo "Succ $plug" || echo "$plug Failed")
    cd $here
done
