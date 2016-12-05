#!/bin/bash

make clean
make vexp_cfg
make

sh mkimg.sh
