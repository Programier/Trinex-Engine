#!/bin/bash

dir=$(dirname $0)
export LD_LIBRARY_PATH=$dir/TrinexEngine/libs/
$dir/TrinexEngineLauncher
