#!/usr/bin/env python

from ConfigParser import ConfigParser, Error as ConfigParserError
import os
import os.path
import sys
import re

class SimpleCompiler(object):
    indent = ' ' * 4

    def compile_one(self, fn):
        assert os.path.isfile(fn)

        self.source_fn = fn
        basename = os.path.splitext(fn)[0]
        self.nameh = basename + '.xsd.h'
        self.namecpp = basename + '.xsd.cpp'

        self.source = ConfigParser()
        self.source.read(fn)
        self.outhpp = open(self.nameh, 'w')
        self.outcpp = open(self.namecpp, 'w')

        try:
            self._do_compile()
        except Exception:
            os.unlink(self.nameh)
            os.unlink(self.namecpp)
            raise

    def _do_compile(self):
        self._gen_opening()

        for section in self.source.sections():
            if section == 'META':
                continue

            items = self.source.items(section)
            if len(items) == 1 and items[0][0] == '.typedef':
                self._gen_typedef_decl(section, items[0][1])
                self._gen_struct_impl(section, None)
            else:
                self._gen_struct_decl(section, items)
                self._gen_struct_impl(section, items)

        self._gen_closing()

    def _gen_opening(self):
        hpp, cpp = self.outhpp.write, self.outcpp.write

        try:
            self.include_guard = self.source.get('META', 'include-guard')
        except ConfigParserError:
            self.include_guard = None

        hpp('\n')

        if self.include_guard is not None:
            hpp('#ifndef %s\n' % self.include_guard)
            hpp('#define %s\n' % self.include_guard)
            hpp('\n')
        else:
            hpp('#pragma once\n')

        hpp('#include <memory>\n')
        hpp('#include <string>\n')
        hpp('#include <vector>\n')
        hpp('#include "interface/stream.h"\n')
        hpp('\n')

        self._gen_namespace_opening(self.outhpp)

        cpp('\n')
        cpp('#include "./%s"\n' % os.path.split(self.nameh)[1])
        cpp('#include "common/simple-xsd.h"\n')
        cpp('#include "common/stream-adaptor.h"\n')
        cpp('\n')

    def _gen_closing(self):
        hpp, cpp = self.outhpp.write, self.outcpp.write

        self._gen_namespace_closing(self.outhpp)

        if self.include_guard is not None:
            hpp('#endif  // %s\n\n' % self.include_guard)

    def _gen_typedef_decl(self, typename, typedef):
        hpp = self.outhpp.write

        hpp('%s// %s\n' % (self.indent, typename))
        hpp('%stypedef %s %s;\n' % (self.indent, self._trans_type(typedef), typename))
        hpp('\n')
        hpp('%sbool XsdSerialize(IOStream *, const %s &);\n' % (self.indent, typename))
        hpp('%sbool XsdSerialize(std::ostream &, const %s &);\n' % (self.indent, typename))
        hpp('%sbool XsdUnserialize(IIStream *, %s &);\n' % (self.indent, typename))
        hpp('%sbool XsdUnserialize(std::istream &, %s &);\n' % (self.indent, typename))
        hpp('\n')
        hpp('\n')

    def _gen_struct_decl(self, structname, items):
        hpp = self.outhpp.write

        hpp('%s// %s\n' % (self.indent, structname))
        hpp('%sstruct %s\n' % (self.indent, structname))
        hpp('%s{\n' % (self.indent))
        for variable, typename in items:
            hpp('%s%s%s %s;\n' % (self.indent, self.indent, self._trans_type(typename), variable))
        hpp('%s};\n' % (self.indent))
        hpp('\n')
        hpp('%sbool XsdSerialize(IOStream *, const %s &);\n' % (self.indent, structname))
        hpp('%sbool XsdSerialize(std::ostream &, const %s &);\n' % (self.indent, structname))
        hpp('%sbool XsdUnserialize(IIStream *, %s &);\n' % (self.indent, structname))
        hpp('%sbool XsdUnserialize(std::istream &, %s &);\n' % (self.indent, structname))
        hpp('\n')
        hpp('\n')

    def _gen_struct_impl(self, structname, items):
        cpp = self.outcpp.write

        args = {
            'indent': self.indent,
            'structname': structname,
            'structshortname': structname[:-2] if len(structname) > 2 and structname.endswith('_t') else structname,
            'using_namespace':
                '%susing namespace %s;\n' % (self.indent, self.namespace) if self.namespace is not None else '',
            }

        if items is not None:
            cpp(
'''namespace fasmio { namespace common { namespace simple_xsd {

%(using_namespace)s
%(indent)stemplate <>
%(indent)sstruct field_composer<%(structname)s>
%(indent)s{
%(indent)s%(indent)sstatic bool compose_field(composer &composer, const %(structname)s &data)
%(indent)s%(indent)s{
%(indent)s%(indent)s%(indent)sreturn ''' % args)
            for index, (variable, typename) in enumerate(items):
                if index != 0:
                    cpp('\n%s&& ' % (self.indent * 4))
                cpp('composer.compose_field(data.%s, "%s")' % (variable, variable))
            cpp(''';
%(indent)s%(indent)s}
%(indent)s};

%(indent)stemplate <>
%(indent)sstruct field_parser<%(structname)s>
%(indent)s{
%(indent)s%(indent)sstatic bool parse_field(parser &parser, %(structname)s &data)
%(indent)s%(indent)s{
%(indent)s%(indent)s%(indent)sreturn ''' % args)
            for index, (variable, typename) in enumerate(items):
                if index != 0:
                    cpp('\n%s&& ' % (self.indent * 4))
                cpp('parser.parse_field(data.%s, "%s")' % (variable, variable))
            cpp(''';
%(indent)s%(indent)s}
%(indent)s};

}}}  // namespace fasmio::common::simple_xsd

''' % args)

        self._gen_namespace_opening(self.outcpp)
        cpp(
'''%(indent)sbool XsdSerialize(IOStream *os, const %(structname)s &data)
%(indent)s{
%(indent)s%(indent)sfasmio::common::simple_xsd::composer composer(os);
%(indent)s%(indent)sreturn composer.compose(data, "%(structshortname)s");
%(indent)s}

%(indent)sbool XsdSerialize(std::ostream &os, const %(structname)s &data)
%(indent)s{
%(indent)s%(indent)sfasmio::common::StdOStreamAdaptor adaptor(os);
%(indent)s%(indent)sreturn XsdSerialize(&adaptor, data);
%(indent)s}

%(indent)sbool XsdUnserialize(IIStream *is, %(structname)s &data)
%(indent)s{
%(indent)s%(indent)sfasmio::common::simple_xsd::parser parser(is);
%(indent)s%(indent)sreturn parser.parse(data, "%(structshortname)s");
%(indent)s}

%(indent)sbool XsdUnserialize(std::istream &is, %(structname)s &data)
%(indent)s{
%(indent)s%(indent)sfasmio::common::StdIStreamAdaptor adaptor(is);
%(indent)s%(indent)sreturn XsdUnserialize(&adaptor, data);
%(indent)s}

''' % args)
        self._gen_namespace_closing(self.outcpp)
        cpp('\n')

    def _gen_namespace_opening(self, outf):
        try:
            namespace = self.namespace
        except AttributeError:
            try:
                namespace = self.namespace = self.source.get('META', 'target-namespace')
            except ConfigParserError:
                namespace = self.namespace = None

        if namespace is not None:
            parts = re.compile('::').split(namespace)
            namespace = ' '.join('namespace ' + part + ' {' for part in parts)
            outf.write('%s\n\n' % namespace)

    def _gen_namespace_closing(self, outf):
        namespace = self.namespace
        if namespace is not None:
            parts = re.compile('::').split(namespace)
            outf.write('%s  // namespace %s\n\n' % ('}' * len(parts), namespace))

    _vector = re.compile(r'(?<!std::)\bvector\b')
    _string = re.compile(r'(?<!std::)\bstring\b')
    _optional = re.compile(r'\boptional\b')
    def _trans_type(self, name):
        name = self._vector.sub('std::vector', name)
        name = self._string.sub('std::string', name)
        name = self._optional.sub('std::unique_ptr', name)
        return name

def main():
    try:
        assert len(sys.argv) == 2

        comp = SimpleCompiler()
        comp.compile_one(sys.argv[1])
    except Exception, e:
        # sys.stderr.write('%s: %s\n' % (type(e), e))
        raise

if __name__ == '__main__':
    main()

