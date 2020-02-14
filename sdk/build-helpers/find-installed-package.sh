#!/bin/sh

python3 -c "$(sed s/#/$1/g ${0%.*}.in)"

