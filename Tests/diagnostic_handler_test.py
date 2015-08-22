#!/usr/bin/env python3
import test_support
import os

class DiagnosticHandlerTest(test_support.TestBase):
    def test_compile_warning_in_source_file(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'int Foo()',
                '{',
                '}',
                ''
            ],
            additional_compile_flags = '-Wall',
            expected_errors = [
                {
                    'id': 'compile warning',
                    'severity': 'warning',
                    'msg': "control reaches end of non-void function",
                    'line': '3'
                }
            ])

    def test_compile_error_in_source_file(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo()',
                '{',
                '   return Bar();',
                '}',
                ''
            ],
            additional_compile_flags = '-Wall',
            expected_errors = [
                {
                    'id': 'compile error',
                    'severity': 'error',
                    'msg': "use of undeclared identifier 'Bar'",
                    'line': '3'
                }
            ])

    def test_compile_error_in_fake_header_source(self):
        with test_support.TempBuildDir() as temp_dir:
            os.mkdir(temp_dir + '/foo')
            os.mkdir(temp_dir + '/fake_header_sources')
            os.mkdir(temp_dir + '/fake_header_sources/foo')

            hpp_file_name = temp_dir + '/foo/bar.h'
            test_support.write_file_lines(hpp_file_name, [
                    'Bar Foo();',
                    ''
                ]
            )

            cpp_file_name = temp_dir + '/fake_header_sources/foo/bar.cpp'
            test_support.write_file_lines(cpp_file_name, [
                    '#include "foo/bar.h"'
                ]
            )

            test_support.write_compilation_database(
                build_directory = temp_dir,
                source_file_names = [cpp_file_name],
                additional_compile_flags = '-I' + temp_dir)

            xml_output = test_support.run_colobot_lint(build_directory = temp_dir,
                                                       source_dir = temp_dir,
                                                       source_paths = [cpp_file_name],
                                                       rules_selection = [])
            self.assert_xml_output_match(
                xml_output = xml_output,
                expected_errors = [
                    {
                        'id': 'header file not self-contained',
                        'severity': 'error',
                        'msg': "Including single header file should not result in compile error: unknown type name 'Bar'",
                        'line': '1'
                    }
                ]
            )
