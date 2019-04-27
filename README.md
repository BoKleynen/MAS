# MAS-Project
## Installation

create a vm (i used ubuntu server 19.04) and setup ssh server (only required if you want to work on a vm)

### Installing dependencies
```bash
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install vim gcc g++ python mercurial python-setuptools git qt5-default autoconf cvs bzr unrar gdb valgrind gsl-bin libgsl2 libgsl-dev flex bison libfl-dev tcpdump sqlite sqlite3 libsqlite3-dev libxml2 libxml2-dev doxygen graphviz imagemagick texlive texlive-extra-utils texlive-latex-extra texlive-font-utils texlive-lang-portuguese dvipng latexmk python-sphinx dia
```

### Installing bake
```bash
mkdir repos && cd repos
git clone https://gitlab.com/nsnam/bake.git
```

add the following to `~/.bashrc`, replacing `/foo/bar` with the actual absolute path to your home directory.
```bash
export BAKE_HOME=/foo/bar/repos/bake
export PYTHONPATH=$PYTHONPATH:$BAKE_HOME:$BAKE_HOME/build/lib
export PATH=$PATH:$BAKE_HOME:$BAKE_HOME/build/bin
alias bake=bake.py
```
you can now use `bake` and `bake.py` from everywhere

### Installing ns-3
```bash
mkdir ns-3 && cd ns-3
```

then follow the ns-3 tutorial from `Downloading ns-3 Using Bake`
