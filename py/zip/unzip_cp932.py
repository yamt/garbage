#! /usr/bin/env python

import argparse
import sys
import zipfile

parser = argparse.ArgumentParser()
parser.add_argument("--password", default='')
parser.add_argument("--encoding", default='cp932')
parser.add_argument("file")
args = parser.parse_args()

encoding = args.encoding
password = bytes(args.password, encoding='ascii')
file = args.file

with zipfile.ZipFile(file, metadata_encoding=encoding) as zip:
    zip.extractall(pwd=password)
