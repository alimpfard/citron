plugs=("curl" "gmp" "json" "sdl" "keyboardmods" "tcl")

has_citron=$(echo "#include <Citron/citron.h>" | gcc -E -)
has_citron=$?
makefvar=""
here=`pwd`
modsdir=""

if [[ $has_citron -eq 0 ]]; then
    #makefvar="-Dexisting=1 -DCTR_STD_EXTENSION_PATH=\"$(ctr --ext)\""
    echo "Had Citron"
fi

echo "built Citron: " $makefvar

if [[ "x$AUTOMAKEBUILD" = "xyes" ]]; then
    modsdir="$AUTOMAKEBUILDDIR"
    mkdir -p $AUTOMAKEBUILDDIR
    echo "Will build into $AUTOMAKEBUILDDIR"
fi

for plug in ${plugs[*]}
do
    echo "building $plug";
    cd "$here/plugins/$plug";
    if [[ -e "./configure" ]]; then
      ./configure
    fi
    echo $(make "EXTRAS=$makefvar" && echo "Succ $plug" || echo "$plug Failed")
    if [[ "x$AUTOMAKEBUILD" = "xyes" ]]; then
      INTO="$AUTOMAKEBUILDDIR/$plug" make install
    fi
    cd $here
done
