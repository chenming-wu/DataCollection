import os
import glob
import cv2
import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', '-i', required=True)
    args = parser.parse_args()

    pics = [os.path.join(root, file) for root, dirs, files in os.walk(args.input) for file in files
            if file[-4:] == '.jpg']

    for pic in pics:
        print pic, pic.replace('-000', '.')
        os.rename(pic, pic.replace('-000', '.'))
        continue
        print pic
        img = cv2.imread(pic)
        ox, oy = img.shape[:2]
        ox /= 2
        oy /= 2
        img = img[ox-350:ox+350, oy-350:oy+350]
        cv2.imwrite(pic[:-4] + '.jpg', img)
        os.remove(pic)

