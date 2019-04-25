export BAKE_HOME=/home/vagrant/repos/bake
export PATH=$PATH:$BAKE_HOME:$BAKE_HOME/build/bin
export PYTHONPATH=$PYTHONPATH:$BAKE_HOME:$BAKE_HOME/build/lib
alias bake=bake.py
 
export NS3DIR="/home/vagrant/ns-3/source/ns-3.29"
 
function waff()
{
  CWD="$PWD"
  cd $NS3DIR >/dev/null
  ./waf --cwd="$CWD" $*
  cd - >/dev/null
}

export -f waff
