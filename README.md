## CMOEA

This repository contains the source code for all the experiments presented in the following publication:

Huizinga J, [Clune J](http://jeffclune.com). (2018). ["Evolving Multimodal Robot Behavior via Many Stepping Stones with the Combinatorial Multi-Objective Evolutionary Algorithm"](https://arxiv.org/abs/1807.03392). arXiv:1807.03392.

**If you use this software in an academic article, please consider citing:**

    @article{huizinga2021evolving,
        title={Evolving multimodal robot behavior via many stepping stones with the combinatorial multi-objective evolutionary algorithm},
        author={Huizinga, Joost and Clune, Jeff},
        journal={Evolutionary computation},
        pages={1--34}
        year={2021}
    }

## 1. Installation

For ease of reproducing the experiments, the code includes the [sferes2](https://github.com/sferes2/sferes2) framework and all modules that were used.


The robotics experiments rely on the Bullet physics engine. Different version of bullet may work, but we recommend bullet 2.87 compiled with demos and double precision

And configure and build the experiment:

    ./waf configure --bullet-double-precision
    ./waf --exp cmoea
    ./waf --exp modularity

The function and maze domain experiments can be found in:

    build/exp/cmoea

The robotics experiments can be found in:

    build/exp/modularity

## 2. Licenses
The code in this repository is licensed under the CeCILL License.
