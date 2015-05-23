#!/usr/bin/env python

import httplib
import StringIO
import sys
import time
import urllib

sys.path.append('../../scripts')
import task_meta

def test(count = 3):
    task = '''<request>
                <md5>020861c8c3fe177da19a7e9539a5dbac</md5>
                <prefix>abcd</prefix>
                <charset>abcdefg</charset>
                <max_length>8</max_length>
              </request>'''

    task_meta.push_tasks("localhost", "md5-guesser", "guess", [task] * count)
    print 'Requested', count
    time.sleep(count * 0.02)
    tasks = task_meta.pop_tasks("localhost", "md5-guesser", "answer")
    print 'Responsed', len(tasks)
    assert len(tasks) == count
    print

if __name__ == '__main__':
    try:
        for n in range(1,50,3):
            test(n)
    except KeyboardInterrupt:
        pass

