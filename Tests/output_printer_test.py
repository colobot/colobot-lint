import test_support
import os

class OutputPrinterTest(test_support.TestBase):
    def test_output_filter(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void deleteMeOne(int* x)',
                '{',
                '   delete x;',
                '}',
                'void deleteMeTwo(int* x)',
                '{',
                '   delete x;',
                '}'
            ],
            additional_options = ['-output-filter', 'src.cpp:1:4'],
            rules_selection = ['NakedDeleteRule'],
            expected_errors = [
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'int'",
                    'line': '3'
                }
            ])
