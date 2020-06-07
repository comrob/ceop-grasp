#
# Date:      2020/01/31 10:33
# Author:    Petra Stefanikova, Petr Vana, Jan Faigl
#

# download and install the algorithm library
sudo apt install git
git clone https://github.com/comrob/crl.git crl
cd crl
git reset --hard 543f5b7fe23f946312531ed62bf533d86daca515

./install.sh

# install libraries needed for compiling
sudo apt install -y ccache
sudo apt install libcairo2-dev liblog4cxx-dev libboost-all-dev
