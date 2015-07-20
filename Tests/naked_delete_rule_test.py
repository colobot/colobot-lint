#!/usr/bin/env python3
import unittest
import test_support
import sys

class TestNakedDeleteRule(test_support.TestBase):
    def test_naked_delete_with_builtin_type(self):
        self.assert_colobot_lint_result(
            source_file_text = 'void deleteMe(int* x)\n{\n delete x;\n}\n',
            expected_errors = [
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'int'",
                    'line': '3'
                }
            ])

    def test_naked_delete_with_user_type(self):
        self.assert_colobot_lint_result(
            source_file_text = 'class Foo\n{};\nvoid deleteMe(Foo* x)\n{\n delete x;\n}',
            expected_errors = [
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'class Foo'",
                    'line': '5'
                }
            ])

    def test_naked_delete_with_array_type(self):
        self.assert_colobot_lint_result(
            source_file_text = 'void deleteMe(char* x)\n{\ndelete[] x;\n}',
            expected_errors = [
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'char'",
                    'line': '3'
                }
            ])

if __name__ == '__main__':
    if len(sys.argv) >= 2:
        test_support.colobot_lint_exectuable = sys.argv.pop(1)

    unittest.main()