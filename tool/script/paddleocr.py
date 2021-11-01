#!/usr/bin/env python

import sys
import requests
import base64
import json
import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFont

image_fname = 'test.jpg'
image_file = open(image_fname, 'rb')
image_base64 = base64.b64encode(image_file.read()).decode()
image_file.close()

req_dict = {}
req_dict['images'] = [image_base64]
req_str = json.dumps(req_dict)

resp = requests.post('http://localhost:8868/predict/ocr_system', json=req_dict)

resp_str = resp.content
resp_dict = json.loads(resp_str)

ocr_results = resp_dict['results'][0]

image = Image.open(image_fname)
drawer = ImageDraw.ImageDraw(image)
font_size = 30
font = ImageFont.truetype('NotoSerifCJK-Bold.ttc', font_size)
for ocr_result in ocr_results:
    points = ocr_result['text_region']
    drawer.polygon([tuple(i) for i in points], outline=(255,0,0))
    drawer.text((points[0][0], points[0][1]-font_size), ocr_result['text'], font=font, fill=(255,0,0))

image.show()
image.save('output.jpg')
