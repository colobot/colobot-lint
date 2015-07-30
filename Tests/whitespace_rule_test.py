#!/usr/bin/env python3
import test_support
import os

class TestWhitespaceRule(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['WhitespaceRule'])
        self.set_default_error_id('whitespace')
        self.set_default_error_severity('style')

    def test_correct_whitespace(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo()',
                '{',
                '    int x = 0;',
                '}',
                ''
            ],
            expected_errors = [])

    def test_whitespace_at_end_of_line(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo()',
                '{',
                '    int x = 0;   ',
                '}',
                ''
            ],
            expected_errors = [
                {
                    'msg': 'Whitespace at end of line',
                    'line': '3'
                }
            ])

    def test_tab_character(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo()',
                '{',
                '\tint x = 0;',
                '\t\tint y = 0;',
                '}',
                ''
            ],
            expected_errors = [
                {
                    'msg': 'Tab character is not allowed as whitespace',
                    'line': '3'
                },
                {
                    'msg': 'Tab character is not allowed as whitespace',
                    'line': '4'
                }
            ])

    def test_dos_style_line_endings(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo()\r',
                '{\r',
                '    int x = 0;\r',
                '}',
                ''
            ],
            expected_errors = [
                {
                    'msg': 'File seems to have DOS style line endings',
                    'line': '1'
                }
            ])

    def test_no_newline_at_end_of_file(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo()',
                '{',
                '    int x = 0;',
                '}'
            ],
            expected_errors = [
                {
                    'msg': 'File should end with newline',
                    'line': '3'
                }
            ])

    def test_whitespace_error_in_fake_header_mode(self):
        with test_support.TempBuildDir() as temp_dir:
            os.mkdir(temp_dir + '/foo')
            os.mkdir(temp_dir + '/fake_header_sources')
            os.mkdir(temp_dir + '/fake_header_sources/foo')

            # report violations in this header
            hpp_file_name = temp_dir + '/foo/bar.h'
            test_support.write_file_lines(hpp_file_name, [
                    'void deleteMe(int* x)',
                    '{',
                    '  delete x;  ',
                    '}',
                    ''
                ]
            )

            # but not in this one
            hpp_file_name = temp_dir + '/foo/baz.h'
            test_support.write_file_lines(hpp_file_name, [
                    'int* createMe()    ',
                    '{',
                    '  return new int;',
                    '}'
                ]
            )

            # and not in main cpp module
            cpp_file_name = temp_dir + '/fake_header_sources/foo/bar.cpp'
            test_support.write_file_lines(cpp_file_name, [
                    '#include "foo/bar.h"',
                    '#include "foo/baz.h"     '
                ]
            )

            test_support.write_compilation_database(
                build_directory = temp_dir,
                source_file_names = [cpp_file_name],
                additional_compile_flags = '-I' + temp_dir)

            xml_output = test_support.run_colobot_lint(build_directory = temp_dir,
                                                       source_dir = temp_dir,
                                                       source_paths = [cpp_file_name],
                                                       rules_selection = ['WhitespaceRule'])
            self.assert_xml_output_match(
                xml_output = xml_output,
                expected_errors = [
                    {
                        'id': 'whitespace',
                        'severity': 'style',
                        'msg': "Whitespace at end of line",
                        'line': '3'
                    }
                ]
            )

if __name__ == '__main__':
    test_support.main()