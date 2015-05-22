#!/bin/bash
cat $0 | tail -n +4 | (mkdir -p /opt/fasmio/ && cd /opt/fasmio && tar xfz -)
exit $?
