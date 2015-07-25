#!/usr/bin/env python3
import test_support
from test_support import TempBuildDir, write_file_lines, write_compilation_database, run_colobot_lint
import os

class TestSourceLocationHelper(test_support.TestBase):
    def test_ignore_macro_body_expansion(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#define DEFINE_FUNC() \\',
                '  void deleteMe(int* x) \\',
                '  { \\',
                '     delete x; \\',
                '  }',
                'DEFINE_FUNC();'
            ],
            expected_errors = [],
            rules_selection = ['NakedDeleteRule'])

    def test_ignore_macro_argument_expansion(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#define DEFINE_FUNC_WITH_INSTR(instr) \\',
                '  void deleteMe(int* x) \\',
                '  { \\',
                '     instr; \\',
                '  }',
                'DEFINE_FUNC_WITH_INSTR(delete x);'
            ],
            expected_errors = [],
            rules_selection = ['NakedDeleteRule'])

    def test_fake_header_source(self):
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

            xml_output = run_colobot_lint(build_directory = temp_dir,
                                          source_paths = [cpp_file_name],
                                          rules_selection = ['NakedNewRule', 'NakedDeleteRule'])
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
    test_support.main()