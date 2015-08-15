#!/usr/bin/env python3
import test_support
import os

class ImplicitBoolCastRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['ImplicitBoolCastRule'])
        self.set_default_error_id('implicit bool cast')
        self.set_default_error_severity('warning')

    def test_implicit_cast_int_to_bool(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '  long int foo;',
                '  bool Bar()',
                '  {',
                '     return foo;',
                '  }',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Implicit cast 'long' -> bool",
                    'line': '6'
                }
            ])

    def test_implicit_cast_bool_to_int(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '  bool foo;',
                '  long Bar()',
                '  {',
                '     return foo;',
                '  }',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Implicit cast bool -> 'long'",
                    'line': '6'
                }
            ])

    def test_implicit_cast_float_to_bool(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'bool Foo()',
                '{',
                '  return 0.0;',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Implicit cast 'double' -> bool",
                    'line': '3'
                }
            ])

    def test_implicit_cast_bool_to_float(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'float Foo()',
                '{',
                '  return false;',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Implicit cast bool -> 'float'",
                    'line': '3'
                }
            ])

    def test_implicit_cast_pointer_to_bool(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '  int* foo;',
                '  int Bar()',
                '  {',
                '     if (foo) return 10;',
                '     else return 20;'
                '  }',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Implicit cast 'int *' -> bool",
                    'line': '6'
                }
            ])

    def test_ignore_comparison_of_bools(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'bool BoolsEqual(bool a, bool b)',
                '{',
                '   return a == b;',
                '}',
                'bool BoolsNotEqual(bool a, bool b)',
                '{',
                '   return a != b;',
                '}'
            ],
            expected_errors = [])

if __name__ == '__main__':
    test_support.main()