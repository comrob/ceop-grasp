#
# Date:      2020/01/31 10:33
# Author:    Petra Stefanikova, Petr Vana, Jan Faigl
#

make

./grasp_ceop \
    --gui=cairo \
    --problem=./etc/op-chao_set64-rad0.5.txt \
    --budget=40 \
    --output=results.example \
    --waypoint-optimization=0 \
    --wait-to-click=0
    