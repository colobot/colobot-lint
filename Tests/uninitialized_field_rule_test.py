#!/usr/bin/env python3
import test_support
from test_support import TempBuildDir, write_file_lines, write_compilation_database, run_colobot_lint
import os

class TestUnintializedFieldRule(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['UninitializedFieldRule'])
        self.set_default_error_id('uninitialized field')
        self.set_default_error_severity('error')

    def test_struct_with_field_initialization_in_declaration(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    int x = 0;',
                '    float y = 0.0f;',
                '    bool z = true;',
                '};'
            ],
            expected_errors = [])

    def test_struct_with_one_field_uninitialized_in_declaration(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    int x = 0;',
                '    float y;',
                '    bool z = true;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Struct 'Foo' field 'y' remains uninitialized",
                    'line': '1'
                }
            ])

    def test_struct_with_constructor_initialization_list(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo() : x(0), y(0.0f), z(true) {}',
                '    int x;',
                '    float y;',
                '    bool z;',
                '};'
            ],
            expected_errors = [])

    def test_struct_with_two_fields_uninitialized_in_constructor_initialization_list(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo() : y(0.0f), z(true) {}',
                '    int x;',
                '    float y;',
                '    bool z;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Struct 'Foo' field 'x' remains uninitialized in constructor",
                    'line': '3'
                }
            ])

    def test_struct_with_fields_initialized_inside_constructor(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo() { x = 0; y = 0.0f; z = true; }',
                '    int x;',
                '    float y;',
                '    bool z;',
                '};'
            ],
            expected_errors = [])

    def test_struct_with_fields_with_default_constructors(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                '#include <complex>',
                'struct Bar',
                '{',
                '   int x;',
                '   Bar() : x(0) {}',
                '};',
                'struct Foo',
                '{',
                '    std::string x;',
                '    std::complex<double> y;',
                '    Bar z;',
                '};'
            ],
            expected_errors = [])

    def test_struct_with_field_without_default_constructor(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                '#include <complex>',
                'struct Bar',
                '{',
                '   int x;',
                '};',
                'struct Foo',
                '{',
                '    std::string x;',
                '    std::complex<double> y;',
                '    Bar z;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Struct 'Bar' field 'x' remains uninitialized",
                    'line': '3'
                },
                {
                    'msg': "Struct 'Foo' field 'z' remains uninitialized",
                    'line': '7'
                }
            ])

    def test_class_with_multiple_constructors_one_constructor_without_one_field_assignment(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo() : z(true) {}',
                '    Foo(int xx) { x = xx; z = false; }',
                '    Foo(float yy) : y(yy) {}',
                '    int x = 0;',
                '    float y = 0.0f;',
                '    bool z;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Struct 'Foo' field 'z' remains uninitialized in constructor",
                    'line': '5'
                }
            ])

    def test_ignore_union(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'union Foo',
                '{',
                '    int anything;',
                '    float anythingAtAll;',
                '};'
            ],
            expected_errors = [])

    def test_ignore_anonymous_union_or_struct(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    struct',
                '    {',
                '        int x;',
                '    };',
                '    union',
                '    {',
                '        int anything;',
                '        float anythingAtAll;',
                '    };',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Struct '' field 'x' remains uninitialized",
                    'line': '3'
                }
            ])

    def test_constructor_declared_but_not_defined_in_fake_header_source(self):
        with TempBuildDir() as temp_dir:
            os.mkdir(temp_dir + '/foo')
            os.mkdir(temp_dir + '/fake_header_sources')
            os.mkdir(temp_dir + '/fake_header_sources/foo')

            cpp_file_name = temp_dir + '/fake_header_sources/foo/bar.cpp'
            write_file_lines(cpp_file_name, [
                    '#include "foo/bar.h"'
                ]
            )

            hpp_file_name = temp_dir + '/foo/bar.h'
            write_file_lines(hpp_file_name, [
                    'class NoConstructor',
                    '{',
                    '   int x;',
                    '};',
                    'class ConstructorDefined',
                    '{',
                    '   ConstructorDefined() {}',
                    '   int x;',
                    '};',
                    'class NotDefinedConstructor',
                    '{',
                    '   NotDefinedConstructor();',
                    '   int x;',
                    '};'
                ]
            )

            write_compilation_database(
                build_directory = temp_dir,
                source_file_names = [cpp_file_name],
                additional_compile_flags = '-I' + temp_dir)

            xml_output = run_colobot_lint(build_directory = temp_dir,
                                          source_dir = temp_dir,
                                          source_paths = [cpp_file_name],
                                          rules_selection = ['UninitializedFieldRule'])
            self.assert_xml_output_match(
                xml_output = xml_output,
                expected_errors = [
                    {
                        'msg': "Class 'NoConstructor' field 'x' remains uninitialized",
                        'line': '1'
                    },
                    {
                        'msg': "Class 'ConstructorDefined' field 'x' remains uninitialized in constructor",
                        'line': '7'
                    }
                ]
            )

if __name__ == '__main__':
    test_support.main()