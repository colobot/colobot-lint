#!/usr/bin/env python3
import unittest
import test_support
import sys

class TestUnintializedFieldRule(test_support.TestBase):
    def setUp(self):
        self.set_rules_selection(['UninitializedFieldRule'])

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
                    'id': 'uninitialized field',
                    'severity': 'error',
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
                    'id': 'uninitialized field',
                    'severity': 'error',
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
                    'id': 'uninitialized field',
                    'severity': 'error',
                    'msg': "Struct 'Bar' field 'x' remains uninitialized",
                    'line': '3'
                },
                {
                    'id': 'uninitialized field',
                    'severity': 'error',
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
                    'id': 'uninitialized field',
                    'severity': 'error',
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

if __name__ == '__main__':
    test_support.main()