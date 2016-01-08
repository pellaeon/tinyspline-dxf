#!/usr/bin/env python
# -*- coding: utf-8 -*-

import dxfgrabber

DEBUG = True
#DEBUG = False

def log(info):
    if DEBUG:
        print(info)

def euclidean_distance(point1, point2):
    total = 0

    for count, (pos1, pos2) in enumerate(zip(point1, point2)):
        total += (pos2 - pos1)**2

    count += 1
    total = total ** (1/count)

    return total


def ellipse(a, b):
    h = ((a-b)**2) / ((a+b)**2)
    result = 3.14159265358979323846 * (a+b) * (1 + 3 * h / (10 + (4 - 3 * h) ** 0.5))
    return result

def rdxf(filename):
    dxf = dxfgrabber.readfile(filename)

    total_length = 0

    ########################################
    # Info
    ########################################

    log("DXF version: {}".format(dxf.dxfversion))

    # dict of dxf header vars
    log('header_var_coun: {}'.format(len(dxf.header)))

    # collection of layer definitions
    log('layer_count: {}'.format(len(dxf.layers)))

    # dict like collection of block definitions
    log('block_definition_count'.format(len(dxf.blocks)))

    # list like collection of entities
    log('entity_count: {}'.format(len(dxf.entities)))
    log('')

    ########################################
    # Entities
    ########################################

    for e in dxf.entities:

        typename = e.dxftype

        log('=' * 20)
        log('DXF Entity: {}\n'.format(typename))

        if typename == 'LWPOLYLINE':

            #print('LWPolyline is closed? {}\n'.format(e.closed))

            length = None
            ppoint = (0, 0)

            for p in e.points:

                log(p)

                if length is None:
                    length = 0
                    ppoint = p
                    continue

                length += euclidean_distance(ppoint, p)
                ppoint = p

            log('Length = {}\n'.format(length))

            total_length += length

        elif typename == 'LINE':

            log('LINE on layer: {}\n'.format(e.dxf.layer))
            log('start point: {}\n'.format(e.dxf.start))
            log('end point: {}\n'.format(e.dxf.end))

            length = euclidean_distance(e.start, e.end)

            log('Length = {}\n'.format(length))

            total_length += length

        elif typename == 'ELLIPSE':

            a = euclidean_distance((0, 0), e.majoraxis)
            b = a * e.ratio

            log('center: {}'.format(e.center))
            log('majoraxis: {}'.format(e.majoraxis))
            log('a: {}'.format(a))
            log('b: {}'.format(b))

            length = ellipse(a, b)

            log('distance: {}'.format(length))

            total_length += length

        elif typename == 'SPLINE':

            print(e.__dict__)
            pass

        else:

            print('Not Implemented')

        log('=' * 20)

    speed = 4

    log('{} mm / {} mmps = {} s'.format(total_length, speed, total_length / speed))

if __name__ == '__main__':
    import sys
    rdxf(sys.argv[1])
