# MAS-Project
## Installation

### VM setup
download vagrant

```bash
vagrant up
```

```bash
vagrant ssh
```

### Installing bake
```bash
mkdir repos && cd repos
git clone https://gitlab.com/nsnam/bake.git
```

add the following to `~/.bashrc`, replacing `/foo/bar` with the actual absolute path to your home directory.
```bash
export BAKE_HOME=/home/vagrant/repos/bake
export PYTHONPATH=$PYTHONPATH:$BAKE_HOME:$BAKE_HOME/build/lib
export PATH=$PATH:$BAKE_HOME:$BAKE_HOME/build/bin
alias bake=bake.py
```
you can now use `bake` and `bake.py` from everywhere

### Installing ns-3
I put my ns-3 installation under `~/repos/ns-3`

then follow the ns-3 tutorial from `Downloading ns-3 Using Bake`

add the following to your .bashrc
```bash
export NS3DIR=/home/vagrant/repos/ns-3/source/ns-3.29
```

to be able to easyly run scripts execute
```bash
ln -s /vagrant/src $NS3DIR/mas
```
