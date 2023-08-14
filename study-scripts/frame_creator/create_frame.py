import sys
import cv2
import time
import numpy as np
import os

def change_brightness(img, value=30):
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    h, s, v = cv2.split(hsv)
    v = cv2.add(v,value)
    v[v > 255] = 255
    v[v < 0] = 0
    final_hsv = cv2.merge((h, s, v))
    img = cv2.cvtColor(final_hsv, cv2.COLOR_HSV2BGR)
    return img

i = 0
qrCrode = cv2.imread("marker.png")
scale_percent = 100

width = int(qrCrode.shape[1] * scale_percent / 100)
height = int(qrCrode.shape[0] * scale_percent / 100)

dsize = (width, height)

qrCrode = cv2.resize(qrCrode, dsize)

h = qrCrode.shape[0]
w = qrCrode.shape[1]

og = cv2.imread("frame.png")
x1 = cv2.imread("frame.png")
x2 = cv2.imread("frame.png")
l = 60
r_l = l * 0.2126
g_l = l * 0.7152
b_l = l * 0.0722

xOffset = 500
yOffset = 150

for y in range(0, og.shape[0]):
    print(y/og.shape[0])
    for x in range(0, og.shape[1]):
        rl = r_l
        gl = g_l
        bl = b_l

        r, g,b = og[y][x]

        r = (r * (235) / 255) + 10
        g = (g * (235) / 255) + 10
        b = (b * (235) / 255) + 10

        if(r + rl > 255): 
            rl = 255-r
        if(g + gl > 255): 
            gl = 255-g
        if(b + bl > 255): 
            bl = 255-b


        if(r - rl < 0):
            rl = 0+r
        if(g - gl < 0):
            gl = 0+g
        if(b - bl < 0):
            bl = 0+b

        x1[y][x] = (r+rl, g+gl, b+bl)
        x2[y][x] = (r-rl, g-gl, b-bl)


for y in range(0,h):
    print(y/h)
    for x in range(0,w):

        rl = r_l
        gl = g_l
        bl = b_l

        r, g,b = og[y + yOffset][x + xOffset]

        r = (r * (235) / 255) + 10
        g = (g * (235) / 255) + 10
        b = (b * (235) / 255) + 10

        if(r + rl > 255): 
            rl = 255-r
        if(g + gl > 255): 
            gl = 255-g
        if(b + bl > 255): 
            bl = 255-b


        if(r - rl < 0):
            rl = 0+r
        if(g - gl < 0):
            gl = 0+g
        if(b - bl < 0):
            bl = 0+b

        if np.sum(qrCrode[y][x]) > 200:
            x1[y + yOffset][x + xOffset] = (r-rl, g-gl, b-bl)
            x2[y + yOffset][x + xOffset] = (r+rl, g+gl, b+bl)
        else:
            x1[y + yOffset][x + xOffset] = (r+rl, g+gl, b+bl)
            x2[y + yOffset][x + xOffset] = (r-rl, g-gl, b-bl)

cv2.imwrite("output1.png", x1)
cv2.imwrite("output2.png", x2)

#Y = 0.2126 R + 0.7152 G + 0.0722 B

#video = cv2.VideoWriter('video.avi', 0, 120, (width,height))

#for x in range(0,1200):
#    clear = lambda: os.system('cls')
#    clear()
#    print(x/1200)

#    video.write(cv2.imread("o1.png"))
#    video.write(cv2.imread("o2.png"))

#video.release()

def RGBtoLUM(colorChannel):
    if colorChannel <= 0.04045:
        return colorChannel / 12.92
    else:
        return pow(((colorChannel + 0.055)/1.055),2.4)