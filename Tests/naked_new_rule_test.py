#!/usr/bin/env python3
import test_support

class NakedNewRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['NakedNewRule'])
        self.set_default_error_id('naked new')
        self.set_default_error_severity('warning')

    def test_naked_new_with_builtin_type(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'int* x = new int;'
            ],
            expected_errors = [
                {
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
                    'msg': "Naked new called with type 'char'",
                    'line': '1'
                }
            ])
