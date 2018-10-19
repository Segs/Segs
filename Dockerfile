FROM ubuntu:bionic

RUN apt-get update && apt-get install sudo

COPY docker/scripts/segs.sh /tmp
RUN /tmp/segs.sh install_deps_ubuntu_bionic

VOLUME /segs
WORKDIR /segs

ENV PATH="/segs/docker/scripts:${PATH}"
