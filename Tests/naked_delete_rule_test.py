#!/usr/bin/env python3
import test_support

class NakedDeleteRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['NakedDeleteRule'])
        self.set_default_error_id('naked delete')
        self.set_default_error_severity('warning')

    def test_naked_delete_with_builtin_type(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void deleteMe(int* x)',
                '{',
                '  delete x;',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Naked delete called on type 'int'",
                    'line': '3'
                }
            ])

    def test_naked_delete_with_user_type(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{};',
                'void deleteMe(Foo* x)',
                '{',
                '  delete x;',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Naked delete called on type 'class Foo'",
                    'line': '5'
                }
            ])

    def test_naked_delete_with_array_type(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void deleteMe(char* x)',
                '{',
                '  delete[] x;',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Naked delete called on type 'char'",
                    'line': '3'
                }
            ])
