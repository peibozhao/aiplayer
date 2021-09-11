#!/usr/bin/env python

import sys
import cv2
import argparse


def plot_one_rect(xyxy, img):
    return cv2.rectangle(img, (xyxy[0], xyxy[1]), (xyxy[2], xyxy[3]), (255, 0, 0), 3)


def main():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('--rect', type=str, required=False)
    arg_parser.add_argument('--Rect', type=str, required=False)
    args = arg_parser.parse_args()

    rects = []
    if args.rect != None:
        rects.append([int(i) for i in args.rect.split(',')])
    if args.Rect != None:
        with open(args.Rect) as f:
            for line in f:
                rects.append([int(float(i)) for i in line[:-1].split(',')])
    img = cv2.imread('./828706969.jpg')
    #  img = cv2.resize(img, (640, 320))
    for rect in rects:
        plot_one_rect(rect, img)
    cv2.imwrite('output.jpg', img)


main()
