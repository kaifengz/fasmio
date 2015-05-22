#!/usr/bin/env python

import urllib
import xml.dom.minidom

def main():
    resp = urllib.urlopen("http://localhost/registry/nodes").read()
    print xml.dom.minidom.parseString(resp).toprettyxml()

if __name__ == '__main__':
    main()

