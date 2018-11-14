FROM ubuntu:bionic

COPY docker/scripts/segs.sh /tmp
RUN /tmp/segs.sh install_deps_ubuntu

VOLUME /segs
WORKDIR /segs

ENV PATH="/segs/docker/scripts:${PATH}"
