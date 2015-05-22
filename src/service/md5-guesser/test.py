#!/usr/bin/env python

import httplib
import StringIO
import time
import urllib

TLV_ASCII_ONLY = False

def compose_int(n):
    if TLV_ASCII_ONLY:
        if n < 128:
            return '%02X' % n
        else:
            return '%08X' % (n | 0x80000000)
    else:
        if n < 128:
            return chr(n)
        else:
            return chr((n >> 24) | 0x80) + chr((n >> 16) & 0xFF) + chr((n >>  8) & 0xFF) + chr(n & 0xFF)

def parse_int(s):
    if TLV_ASCII_ONLY:
        n = s.read(2)
        assert len(n) == 2
        n = int(n, 16)
        if n < 128:
            return n
        else:
            t = s.read(6)
            assert len(t) == 6
            return ((n - 128) << 24) + int(t, 16)
    else:
        n = s.read(1)
        assert len(n) == 1
        n = ord(n)
        if n < 128:
            return n
        else:
            n -= 128
            t = s.read(3)
            assert len(t) == 3
            for x in t:
                n <<= 8
                n += ord(x)
            return n

def compose_request_data(tasks):
    s = StringIO.StringIO()
    s.write('K' + compose_int(4) + 'root')
    for task in tasks:
        length = len(task)
        s.write('V' + compose_int(4) + 'meta' + compose_int(0))
        s.write('V' + compose_int(4) + 'task')
        s.write(compose_int(length))
        s.write(task)
        s.write(compose_int(0))
    s.write('E')
    return s.getvalue()

def push_tasks(host, service, slot, tasks):
    url = 'http://%s/%s/%s' % (host, service, slot)
    data = compose_request_data(tasks)
    print 'len(data) = %d' % len(data)
    # print 'data = %s' % data
    urllib.urlopen(url, data).read()

def parse_response_data(resp):
    head, tail = 'K' + compose_int(4) + 'root', 'E'
    assert resp.startswith(head), resp
    assert resp.endswith(tail), resp

    s = StringIO.StringIO(resp[len(head):-len(tail)])
    def read_value(s):
        value = ''
        while True:
            part_len = parse_int(s)
            if part_len == 0:
                break
            part = s.read(part_len)
            assert len(part) == part_len
            value += part
        return value

    empty_meta = 'K' + compose_int(4) + 'meta' + 'E'
    task_head = 'V' + compose_int(4) + 'task'
    tasks = []
    while True:
        t = s.read(len(empty_meta))
        if len(t) == 0:
            break

        assert t == empty_meta
        meta = None

        t = s.read(len(task_head))
        task = read_value(s)
        #assert t == task_head
        #task_len = parse_int(s)
        #task = s.read(task_len)
        #assert len(task) == task_len

        tasks.append((meta, task))
    return tasks

def pop_tasks(host, service, slot):
    url = 'http://%s/%s/%s' % (host, service, slot)
    pop_req = '''<pop_request>
                    <count_limit>256</count_limit>
                 </pop_request>'''
    conn = httplib.HTTPConnection(host)
    conn.request('PUT', url, pop_req)
    resp = conn.getresponse().read()
    print 'len(resp) = %d' % len(resp)
    print 'resp = %s' % resp
    return parse_response_data(resp)

def test(count = 3):
    task = '''<request>
                <md5>020861c8c3fe177da19a7e9539a5dbac</md5>
                <prefix>abcd</prefix>
                <charset>abcdefg</charset>
                <max_length>8</max_length>
              </request>'''

    push_tasks("localhost", "md5-guesser", "guess", [task] * count)

    time.sleep(count * 0.2)

    tasks = pop_tasks("localhost", "md5-guesser", "answer")
    assert len(tasks) == count

if __name__ == '__main__':
    try:
        for n in range(1,50,3):
            test(n)
    except KeyboardInterrupt:
        pass

