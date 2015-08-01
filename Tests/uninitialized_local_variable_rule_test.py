#!/usr/bin/env python3
import test_support

class UnintializedLocalVariableRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['UninitializedLocalVariableRule'])
        self.set_default_error_id('uninitialized local variable')
        self.set_default_error_severity('error')

    def test_function_with_builtin_variables_defined(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo()',
                '{',
                '    int x = 0, y = 1;',
                '    bool z = true;',
                '}'
            ],
            expected_errors = [])

    def test_function_with_one_builtin_variable_undefined(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo()',
                '{',
                '    int x, y = 1;',
                '    bool z = true;',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Local variable 'x' is uninitialized",
                    'line': '3'
                }
            ])

    def test_function_with_pod_type_undefined(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Bar { int x; };',
                'void Foo()',
                '{',
                '    Bar bar;',
                '    bool z = true;',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Local variable 'bar' is uninitialized",
                    'line': '4'
                }
            ])

    def test_function_with_non_pod_types_undefined(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Bar { int x = 0; };',
                'void Foo()',
                '{',
                '    Bar bar;',
                '    std::string z;',
                '}'
            ],
            expected_errors = [])

    def test_skip_function_parameters(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo(int x, int y, int z);',
                'void Bar(int x, int y, int z) {}',
            ],
            expected_errors = [])

    def test_dont_report_uninitialized_variables_in_old_style_functions(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'void Bar(int &x, int &y, int& z);',
                'void Foo()',
                '{',
                '    int x, y, z;',
                '    float a, b, c',
                '    std::string str;',
                '',
                '    Bar(x, y, z);',
                '    a = b = c = 10.0f;',
                '    str = "123";',
                '}'
            ],
            expected_errors = [
                {
                    'id': 'old style function',
                    'severity': 'warning',
                    'msg': "Function 'Foo' seems to be written in legacy C style: " +
                           "it has uninitialized POD type variables declared far from their point of use ('x', 'y', 'z', 'a'... and 2 more)",
                    'line': '3'
                }
            ],
            rules_selection = ['OldStyleFunctionRule', 'UninitializedLocalVariableRule'])

if __name__ == '__main__':
    test_support.main()