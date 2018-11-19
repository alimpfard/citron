#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: $0 <packagename> <minimum version>"
    exit 1
fi

pkgname="$1"
minversion="$2"

V="0"

for version in `apt-cache madison $pkgname | awk -F'|' '{print $2}'`; do
    echo "Considering $version"
    if dpkg --compare-versions $version ge $minversion; then
        echo "- Version is at least $minversion"
        if dpkg --compare-versions $version gt $V; then
            echo "- This is the newest version so far"
            V=$version
        else
            echo "- We already have a newer version"
        fi
    else
        echo "- This is older than $minversion"
    fi
done

if [ "$V" = "0" ]; then
    echo "There is no version greater than or equal to $minversion"
    exit 1
fi

echo "Selected version: $V"

echo "Downloading"
apt-get install $pkgname=$V
echo "Done"
