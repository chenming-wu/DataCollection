import os
import argparse
import shutil


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input')
    parser.add_argument('-o', '--output')
    args = parser.parse_args()

    for root, dirs, files in os.walk(args.input):
        for file in files:
            if file.endswith('.jpg'):
                shutil.copy(os.path.join(root, file), args.output)

