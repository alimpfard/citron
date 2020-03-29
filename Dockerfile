FROM ubuntu:bionic
MAINTAINER Ali MohammadPur <devanothertest@gmail.com>

RUN apt-get update && apt-get install -y wget tar libbsd0 libgc-dev libcurl4 build-essential

RUN wget 'https://github.com/alimpfard/citron/releases/download/continuous/release.tar.bz2'
RUN mkdir -p citron && mv release.tar.bz2 citron

WORKDIR /citron

RUN tar xjf release.tar.bz2
# we don't need it as an appimage
# RUN bin/ctr --appimage-extract
# RUN mv squashfs-root/usr/bin/ctr bin/ctr
# RUN rm -rf squashfs-root

RUN (tar cf - .) | (cd .. && tar xf -)

ENV CITRON_EXT_PATH=/share/Citron

ENTRYPOINT ["citron"]
