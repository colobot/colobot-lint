#!/usr/bin/env python3
import unittest
import test_support
import sys

class TestNakedNewRule(test_support.TestBase):
    def test_naked_new_with_builtin_type(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'int* x = new int;'
            ],
            expected_errors = [
                {
                    'id': 'naked new',
                    'severity': 'warning',
                    'msg': "Naked new called with type 'int'",
                    'line': '1'
                }
            ])

    def test_naked_new_with_user_type(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{};',
                'Foo* foo = new Foo();'
            ],
            expected_errors = [
                {
                    'id': 'naked new',
                    'severity': 'warning',
                    'msg': "Naked new called with type 'class Foo'",
                    'line': '3'
                }
            ])

    def test_naked_new_with_array_type(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'char* x = new char[256];'
            ],
            expected_errors = [
                {
                    'id': 'naked new',
                    'severity': 'warning',
                    'msg': "Naked new called with type 'char'",
                    'line': '1'
                }
            ])

if __name__ == '__main__':
    if len(sys.argv) >= 2:
        test_support.colobot_lint_exectuable = sys.argv.pop(1)

    unittest.main()