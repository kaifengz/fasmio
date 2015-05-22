#!/usr/bin/env python

import sys
import urllib
import xml.dom.minidom

def retire(service):
    url = "http://localhost/%s/@retire" % service
    resp = urllib.urlopen(url, data="")
    assert resp.code == 200
    resp.read()

actions = {
    "retire": retire,
}

def main():
    try:
        action = actions[sys.argv[1]]
    except IndexError, KeyError:
        return
    else:
        action(*sys.argv[2:])

if __name__ == '__main__':
    main()

