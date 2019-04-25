apt-get update
apt-get upgrade
apt-get install -y vim

# https://www.nsnam.org/wiki/Installation#Ubuntu.2FDebian.2FMint
apt-get install -y gcc g++ python
apt-get install -y mercurial python-setuptools git
apt-get install -y qt5-default
apt-get install -y autoconf cvs bzr unrar
apt-get install -y gdb valgrind
apt-get install -y gsl-bin libgsl2 libgsl-dev
apt-get install -y flex bison libfl-dev
apt-get install -y tcpdump
apt-get install -y sqlite sqlite3 libsqlite3-dev
apt-get install -y libxml2 libxml2-dev
apt-get install -y doxygen graphviz imagemagick
apt-get install -y texlive texlive-extra-utils texlive-latex-extra texlive-font-utils texlive-lang-portuguese dvipng latexmk
apt-get install -y python-sphinx dia

#install bake
# if ! [ -x "$bake" ]
# then
#   echo "installing bake ..."
#   BAKE_HOME="/home/vagrant/repos/bake"
#   git clone https://gitlab.com/nsnam/bake.git $BAKE_HOME
#   cat >> .bashrc <<EOL
# export BAKE_HOME=$BAKE_HOME
# export PATH=$PATH:$BAKE_HOME:$BAKE_HOME/build/bin
# export PYTHONPATH=$PYTHONPATH:$BAKE_HOME:$BAKE_HOME/build/lib
# alias bake=bake.py
# EOL
#   echo "bake installed ..."
# else
#   echo "bake already installed"
# fi
