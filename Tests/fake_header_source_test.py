#!/usr/bin/env python3
import unittest
import test_support
from test_support import TempBuildDir, write_file_lines, write_compilation_database, run_colobot_lint
import os
import sys

class TestFakeHeader(test_support.TestBase):
    def test_fake_header(self):
        with TempBuildDir() as temp_dir:
            os.mkdir(temp_dir + '/foo')
            os.mkdir(temp_dir + '/fake_header_sources')
            os.mkdir(temp_dir + '/fake_header_sources/foo')

            cpp_file_name = temp_dir + '/fake_header_sources/foo/bar.cpp'
            write_file_lines(cpp_file_name, [
                    '#include "foo/bar.h"',
                    '#include "foo/baz.h"'
                ]
            )

            # report violations in this header
            hpp_file_name = temp_dir + '/foo/bar.h'
            write_file_lines(hpp_file_name, [
                    'void deleteMe(int* x)',
                    '{',
                    '  delete x;',
                    '}'
                ]
            )

            # but not in this one
            hpp_file_name = temp_dir + '/foo/baz.h'
            write_file_lines(hpp_file_name, [
                    'int* createMe()',
                    '{',
                    '  return new int;',
                    '}'
                ]
            )

            write_compilation_database(
                build_directory = temp_dir,
                source_file_names = [cpp_file_name],
                additional_compile_flags = '-I' + temp_dir)

            xml_output = run_colobot_lint(temp_dir, [cpp_file_name])
            self.assert_xml_output_match(
                xml_output = xml_output,
                expected_errors = [
                    {
                        'id': 'naked delete',
                        'severity': 'warning',
                        'msg': "Naked delete called on type 'int'",
                        'line': '3'
                    }
                ]
            )


if __name__ == '__main__':
    if len(sys.argv) >= 2:
        test_support.colobot_lint_exectuable = sys.argv.pop(1)

    unittest.main()