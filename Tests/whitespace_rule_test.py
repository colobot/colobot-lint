import test_support

class WhitespaceRuleTest(test_support.TestBase):
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
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                # report violations in this file:
                'bar.h' : [
                    'void deleteMe(int* x)',
                    '{',
                    '  delete x;  ',
                    '}',
                    ''
                ],
                # but not this one:
                'baz.h' : [
                    'int* createMe()    ',
                    '{',
                    '  return new int;',
                    '}'
                ],
                # and not main file:
                'fake_header_sources/bar.cpp': [
                    '#include "bar.h"',
                    '#include "baz.h"       '
                ]
            },
            compilation_database_files = ['fake_header_sources/bar.cpp'],
            target_files = ['fake_header_sources/bar.cpp'],
            additional_compile_flags = ['-I$TEMP_DIR'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR'],
            expected_errors = [
                {
                    'msg': "Whitespace at end of line",
                    'line': '3'
                }
            ])
