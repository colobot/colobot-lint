#!/usr/bin/env python3
import test_support

class TestWhitespaceRule(test_support.TestBase):
    def setUp(self):
        self.set_rules_selection(['WhitespaceRule'])

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
                    'id': 'whitespace',
                    'severity': 'style',
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
                    'id': 'whitespace',
                    'severity': 'style',
                    'msg': 'Tab character is not allowed as whitespace',
                    'line': '3'
                },
                {
                    'id': 'whitespace',
                    'severity': 'style',
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
                    'id': 'whitespace',
                    'severity': 'style',
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
                    'id': 'whitespace',
                    'severity': 'style',
                    'msg': 'File should end with newline',
                    'line': '3'
                }
            ])

if __name__ == '__main__':
    test_support.main()