# Greedy Randomized Adaptive Search Procedure for Close Enough Orienteering Problem

## Overview
Greedy Randomized Adaptive Search Procedure was extended for solving Close Enough Orienteering Problem. The method has been introduced in [this article](https://doi.org/10.1145/3341105.3374010).

```
@inproceedings{stefanikova20greedy,
  title={Greedy randomized adaptive search procedure for close enough orienteering problem},
  author={{\v{S}}tefan{\'\i}kov{\'a}, Petra and V{\'a}{\v{n}}a, Petr and Faigl, Jan},
  booktitle={Proceedings of the 35th Annual ACM Symposium on Applied Computing},
  pages={808--814},
  year={2020},
  doi={10.1145/3341105.3374010}
}
```

## CEOP Example

![CEOP example](grasp.gif?raw=true)

## Instruction to build from source
The script dowlownds and install all needed libraries.
```sh
./install.sh
```

## Instruction to run GRASP-based algorithm
To run the code, the sources (C++11) need to be compiled using make, see [example.sh](example.sh). (Tested on Ubuntu 18.04).

```sh
make

./grasp_ceop \
    --gui=cairo \
    --problem=./etc/op-chao_set64-rad0.5.txt \
    --budget=40 \
    --output=results.example \
    --waypoint-optimization=0 \
    --wait-to-click=0
```

### Configuration

All possible configurations are shown by `./grasp_ceop -h`. The configuration can be determined at launch by flags or in the [grasp_ceop.cfg](grasp_ceop.cfg) file.

The most important configuration flags:

| FLAG | VALUES | DESCRIPTION |
| --- | --- | --- |
| gui | cairo, none | enable/disable graphics visualization |
| problem | problem file from *./etc* directory | problem desription file (position of nodes) |
| budget | positive number | OP budget |
| waypoint-optimization | 0, 1 | enable/disable waypoint optimization in the Local Search Phase |
| wait-to-click | 0, 1 | enable/disable waiting for click after redrawing new best path |
