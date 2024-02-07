import sys
import cv2
import time
import numpy as np
import os

arucoDict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_1000)
arucoDict2 = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_5X5_1000)
arucoDict3 = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_6X6_1000)

tag = np.zeros((300,300,1), dtype="uint8")
cv2.aruco.generateImageMarker(arucoDict,1, 300, tag, 1)
cv2.imwrite("aruco_1.png", tag)

img = np.full((300,25,1),255, dtype="uint8")
for i in range(5):
    tag = np.zeros((300,300,1), dtype="uint8")
    cv2.aruco.generateImageMarker(arucoDict3,1, 300, tag, 1)
    empty = np.full((300,25,1), 255, dtype="uint8")
    img = np.concatenate([img, tag, empty ], axis=1)

img2 = np.full((25, 1920, 1), 255, dtype="uint8")
empty = np.full((300,270,1), 255, dtype="uint8")
img = np.concatenate([img, empty ], axis=1)

for i in range(3):
 
    empty = np.full((25,1920,1), 255, dtype="uint8")
    img2 = np.concatenate([img2, img, empty ],axis=0)

empty = np.full((80,1920,1), 255, dtype="uint8")
img2 = np.concatenate([img2,empty ],axis=0)

