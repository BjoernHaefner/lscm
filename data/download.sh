#!/bin/bash

script_path="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

wget -P "$script_path" "https://raw.githubusercontent.com/NVIDIA/OptiX_Apps/master/data/cow.obj"
