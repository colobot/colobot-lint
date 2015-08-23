import test_support

class OldStyleNullPointerRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['OldStyleNullPointerRule'])
        self.set_default_error_id('old-style null pointer')
        self.set_default_error_severity('style')

    def test_use_of_nullptr_constant(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'int* x = nullptr;'
            ],
            expected_errors = [])

    def test_use_of_zero_integer_literal(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'char* x = 0;',
                'int Foo()',
                '{',
                '    if (x == 0) return 10;',
                '    return 20;',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Use of old-style zero integer literal as null pointer",
                    'line': '1'
                },
                {
                    'msg': "Use of old-style zero integer literal as null pointer",
                    'line': '4'
                },
            ])
