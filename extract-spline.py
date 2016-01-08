#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy as np
import dxfgrabber

def rdxf(filename):
    dxf = dxfgrabber.readfile(filename)

    for e in dxf.entities:

        typename = e.dxftype
        print(typename)

        if typename == 'SPLINE':

            #print(e.__dict__)
            print('n_ctrlp: %d' % len(e.controlpoints))
            w = e.weights
            for (i, pt) in enumerate(e.controlpoints):
                print('ctrlp: %f %f %f' % (pt[0]*w[i], pt[1]*w[i], pt[2]*w[i]))
            for (index, knot) in enumerate(e.knots):
                print('knot: %f' % (knot))
            print('u_min: %f' % e.knots[0])
            print('u_max: %f' % e.knots[-1])
            pass

        else:

            print('Not Implemented')


if __name__ == '__main__':
    import sys
    rdxf(sys.argv[1])
