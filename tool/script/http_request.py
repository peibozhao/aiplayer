#!/usr/bin/env python

import sys
import requests
import base64
import json
import cv2

image_fname = sys.argv[1]
image_file = open(image_fname, 'rb')
image = image_file.read()
image_base64 = base64.b64encode(image).decode()

req_dict = {}
req_dict['image'] = {}
req_dict['image']['data'] = image_base64
req_str = json.dumps(req_dict)

resp = requests.patch('http://0.0.0.0:8000/blhx/players/123', req_str)

resp_str = resp.content
resp_dict = json.loads(resp_str)
cv_image = cv2.imread(image_fname)
for operation in resp_dict['operations']:
    if operation['type'] == 'click':
        x = operation['x']
        y = operation['y']
        cv_image = cv2.circle(cv_image, (x, y), 5, (0, 0, 255), 5)
        cv_image = cv2.circle(cv_image, (x, y), 20, (0, 0, 255), 4)
        cv2.imwrite('output.jpg', cv_image)

