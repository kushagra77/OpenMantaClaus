import cv2
import numpy as np

def nothing(x):
    pass

# Load image
image = cv2.imread('bags/bucketmasking/comp.jpg')
# image = cv2.imread('bags/buckets_multiple/img_0026.jpg')
# image[:, :, 0] = (image[:, :, 0] * 0.5).astype(np.uint8)
# image = cv2.GaussianBlur(image, (3, 3), 0) # for blurring the image, can use if want to

# Create a window
cv2.namedWindow('image')
default = [[0, 0, 0], [179, 255, 255]]
# default = [0, 0, 196], [55, 132, 255] # [0, 0, 80], [140, 160, 255] # RED
# default = [50, 150, 0], [160, 255, 135] # BLUE
# default = [32, 114, 247], [173, 130, 255]#[150, 100, 110], [190, 150, 200] # YELLOW
default =  [105, 180, 0], [130, 255, 255] # BLUE
default = [50, 0, 0], [85, 255, 255] # YELLOW
default = [120, 0, 0], [179, 255, 255] # RED


# Create trackbars for color change
# Hue is from 0-179 for Opencv
cv2.createTrackbar('HMin', 'image', 0, 179, nothing)
cv2.createTrackbar('SMin', 'image', 0, 255, nothing)
cv2.createTrackbar('VMin', 'image', 0, 255, nothing)
cv2.createTrackbar('HMax', 'image', 0, 179, nothing)
cv2.createTrackbar('SMax', 'image', 0, 255, nothing)
cv2.createTrackbar('VMax', 'image', 0, 255, nothing)

# Set default value for Max HSV trackbars

cv2.setTrackbarPos('HMax', 'image', default[1][0])
cv2.setTrackbarPos('SMax', 'image', default[1][1])
cv2.setTrackbarPos('VMax', 'image', default[1][2])
cv2.setTrackbarPos('HMin', 'image', default[0][0])
cv2.setTrackbarPos('SMin', 'image', default[0][1])
cv2.setTrackbarPos('VMin', 'image', default[0][2])

# Initialize HSV min/max values
hMin = sMin = vMin = hMax = sMax = vMax = 0
phMin = psMin = pvMin = phMax = psMax = pvMax = 0

while(1):
    # Get current positions of all trackbars
    hMin = cv2.getTrackbarPos('HMin', 'image')
    sMin = cv2.getTrackbarPos('SMin', 'image')
    vMin = cv2.getTrackbarPos('VMin', 'image')
    hMax = cv2.getTrackbarPos('HMax', 'image')
    sMax = cv2.getTrackbarPos('SMax', 'image')
    vMax = cv2.getTrackbarPos('VMax', 'image')

    # Set minimum and maximum HSV values to display
    lower = np.array([hMin, sMin, vMin])
    upper = np.array([hMax, sMax, vMax])

    # Convert to HSV format and color threshold
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    
    # y_channel = hsv[:, :, 0]
    # # multiply v channel by 2, cap at 255 to avoid overflow
    
    # hsv[:, :, 2] = cv2.multiply(hsv[:, :, 2], 2.0)
    # erode a little bit to reduce noise, can use if want to
    kernel = np.ones((3, 3), np.uint8)
    # hsv = cv2.cvtColor(hsv, cv2.COLOR_YUV2BGR)
    # hsv = cv2.cvtColor(hsv, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, lower, upper)
    mask = cv2.dilate(mask, kernel, iterations=1)
    result = cv2.bitwise_and(image, image, mask=mask)

    # Print if there is a change in HSV value
    if((phMin != hMin) | (psMin != sMin) | (pvMin != vMin) | (phMax != hMax) | (psMax != sMax) | (pvMax != vMax) ):
        print("[%d, %d, %d], [%d, %d, %d]" % (hMin , sMin , vMin, hMax, sMax , vMax))
        phMin = hMin
        psMin = sMin
        pvMin = vMin
        phMax = hMax
        psMax = sMax
        pvMax = vMax

    # Display result image
    cv2.imshow('image', result)
    cv2.imshow('yuv', hsv)
    if cv2.waitKey(10) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
