
import httplib
import StringIO
import sys
import urllib
import uuid

# TODO: name this file with a more accurate file name

TLV_ASCII_ONLY = False

class TlvComposer(object):
    def __init__(self, stream):
        self.stream = stream
        self._write = stream.write

    def begin_key(self, name):
        assert len(name) < 128, 'To simplify implement, names longer than 127 bytes are not allowed'
        if TLV_ASCII_ONLY:
            name_len = '%02X' % len(name)
        else:
            name_len = chr(len(name))
        self._write('K%s%s' % (name_len, name))

    def end_key(self, name):
        self._write('E')

    def add_value(self, name, value):
        assert len(name) < 128, 'To simplify implement, names longer than 127 bytes are not allowed'
        value = str(value)
        value_len = len(value)

        if TLV_ASCII_ONLY:
            name_len = '%02X' % len(name)
            value_end = '00' if value_len > 0 else ''
            value_len = '%02X' % value_len if value_len < 128 else '%08X' % (0x80000000 | value_len)
        else:
            name_len = chr(len(name))
            value_end = '\0' if value_len > 0 else ''
            if value_len < 128:
                value_len = chr(value_len)
            else:
                value_len = '%c%c%c%c' % ((0x80 | (value_len>>24)), (0xFF & (value_len>>16)), (0xFF & (value_len>>8)), (0xFF & value_len))
        self._write('V%s%s%s%s%s' % (name_len, name, value_len, value, value_end))

class TlvParseError(Exception):
    pass

class TlvParser(object):
    def __init__(self, stream):
        self._stream = stream
        self._stream_read = stream.read

    def parse(self):
        return self._parse_node()

    def _parse_node(self):
        node_type = self._read(1).lower()
        if node_type == 'e':
            return None

        node_name_len = self._get_int()
        node_name = self._read(node_name_len)

        if node_type == 'k':
            children = []
            while True:
                child = self._parse_node()
                if child is None:
                    break
                children.append(child)
            return (node_name, children)
        elif node_type == 'v':
            value = ''
            while True:
                piece_length = self._get_int()
                if piece_length == 0:
                    break
                piece = self._read(piece_length)
                value += piece
            return (node_name, value)
        else:
            raise TlvParseError, "unexpected tag '%s'" % node_type

    def _read(self, length):
        r = self._stream_read(length)
        if len(r) != length:
            raise TlvParseError, 'unexpected end of stream'
        return r

    def _get_int(self):
        if TLV_ASCII_ONLY:
            n = int(self._read(2), 16)
        else:
            n = ord(self._read(1))

        if n >= 0x80:
            n &= 0x7F
            for i in range(3):
                if TLV_ASCII_ONLY:
                    t = int(self._read(2), 16)
                else:
                    t = ord(self._read(1))
                n = (n << 8) + t
        return n

class TaskMeta(object):
    def __init__(self, task_id=None):
        self.task_ids = None if task_id is None else [task_id]
        self.generator_node = ''
        self.generator_service = ''
        self.birth_time = 0.0
        self.need_ack = True
        self.resend_count = 0
        self.resend_time = 0.0

    def serialize(self, composer):
        if self.task_ids:
            composer.begin_key('TaskIds')
            for task_id in self.task_ids:
                composer.add_value('TaskId', task_id)
            composer.end_key('TaskIds')

        composer.begin_key('Generator')
        composer.add_value('Node', self.generator_node)
        composer.add_value('Service', self.generator_service)
        composer.add_value('BirthTime', self.birth_time)
        if not self.need_ack:
            composer.add_value('Node', self.need_ack)
        composer.add_value('ResendCount', self.resend_count)
        if self.resend_count > 0:
            composer.add_value('ResendTime', self.resend_time)
        composer.end_key('Generator')

    def unserialize(self, stream_or_string):
        pass

    @classmethod
    def gen_random(cls):
        return TaskMeta(str(uuid.uuid4()))

def push_tasks(host, service, slot, tasks):
    url = 'http://%s/%s/%s' % (host, service, slot)
    data = StringIO.StringIO()

    composer = TlvComposer(data)
    composer.begin_key('root')
    for task in tasks:
        if hasattr(task, 'meta'):
            meta = task.meta
        else:
            meta = TaskMeta.gen_random()

        composer.begin_key('meta')
        meta.serialize(composer)
        composer.end_key('meta')

        if hasattr(task, 'serialize'):
            if hasattr(task, 'ante_serialize'):
                task.ante_serialize()
            task_stream = StringIO.StringIO()
            task.serialize(task_stream)
            task = task_stream.getvalue()
        else:
            task = str(task)
        composer.add_value('task', task)
    composer.end_key('root')
    data = data.getvalue()

    urllib.urlopen(url, data).read()

def pop_tasks(host, service, slot, task_parser=None):
    url = '/%s/%s' % (service, slot)
    request_data = '<pop_request><count_limit>1024</count_limit></pop_request>'

    conn = httplib.HTTPConnection(host)
    conn.connect()
    conn.request('PUT', url, request_data)
    resp = conn.getresponse()

    parser = TlvParser(resp)
    parsed = parser.parse()
    assert parsed[0] == 'root'
    assert len(parsed[1]) % 2 == 0

    metas = parsed[1][0::2]
    tasks = parsed[1][1::2]
    assert all(meta[0].lower() == 'meta' for meta in metas)
    assert all(task[0].lower() == 'task' for task in tasks)
    meta_tasks = [(meta[1], task[1]) for meta, task in zip(metas, tasks)]
    meta_tasks = [(meta if len(meta) else None, task) for meta, task in meta_tasks]
    # TODO: convert meta string to TaskMeta

    if task_parser is not None:
        meta_tasks = [(meta, task_parser(task)) for meta, task in meta_tasks]
    return meta_tasks

