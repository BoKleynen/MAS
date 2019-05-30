FROM ubuntu:bionic
RUN apt-get update

# General dependencies
RUN apt-get install -y \
  git \
  mercurial \
  wget \
  vim \
  autoconf \
  bzr \
  cvs \
  unrar \
  build-essential \
  clang \
  valgrind \
  gsl-bin \
  libgslcblas0 \
  libgsl-dev \
  flex \
  bison \
  libfl-dev \
  tcpdump \
  sqlite \
  sqlite3 \
  libsqlite3-dev \
  libxml2 \
  libxml2-dev \
  vtun \
  lxc \
  gnuplot

RUN mkdir -p $HOME/repos
WORKDIR $HOME/repos

# bake
RUN git clone https://gitlab.com/nsnam/bake.git
ENV BAKE_HOME=$HOME/repos/bake
ENV PATH=$PATH:$BAKE_HOME:$BAKE_HOME/build/bin
ENV PYTHONPATH=$PYTHONPATH:$BAKE_HOME:$BAKE_HOME/build/lib

# ns-3
RUN mkdir $HOME/repos/ns-3
WORKDIR $HOME/repos/ns-3

RUN bake.py configure -e ns-3.29 && bake.py download

# Cleanup
RUN apt-get clean && \
  rm -rf /var/lib/apt
