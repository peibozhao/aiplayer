#!/usr/bin/env python

import cv2
import sys
import time


def main():
    while True:
        img = cv2.imread(sys.argv[1])
        if isinstance(img, type(None)):
            # Writting
            time.sleep(1)
            continue
        img = cv2.resize(img, (int(img.shape[1]/2), int(img.shape[0]/2)))
        cv2.imshow('show', img)
        cv2.waitKey(1)
        #  time.sleep(1)


if __name__ == '__main__':
    main()
