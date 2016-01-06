#!/bin/sh

for file in real_data/*.dxf;
do
    echo "=========="
    echo $file;
    filename=${file%%.*}
    ../extract-spline.py "${filename}.dxf" > "${filename}.nurbs"
    ./distance.out "${filename}.nurbs"
    echo "=========="
done
