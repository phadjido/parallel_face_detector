import cv2
import numpy as np

# read image
image = 'class57.pgm'
# image = 'chcolat.pgm'
# image = 'f4.pgm'

img = cv2.imread('../pgms/' + image)

# convert to grayscale
gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)

# get a copy
result = img.copy()

# draw bounding boxes for faces
if image == 'class57.pgm':
   faces = [(984, 399, 37, 41), (750, 260, 37, 41), (241, 434, 39, 43)]
elif image == 'chcolat.pgm':
   faces = [(252, 124, 70, 78)]
elif image == 'f4.pgm':
   faces = [(144, 66, 46, 51)]

for face in faces:
    x,y,h,w = face
    cv2.rectangle(result, (x-w//2, y-h//2), (x+w//2, y+h//2), (0, 0, 255), 2)
    print("x,y,w,h:",x,y,w,h)

# save resulting image
cv2.imwrite(image[:-4] + '_result.jpg', result)

# show result
cv2.imshow("bounding_box", result)
cv2.waitKey(0)
cv2.destroyAllWindows()
