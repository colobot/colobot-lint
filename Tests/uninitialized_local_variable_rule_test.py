#!/usr/bin/env python3
import test_support

class TestUnintializedLocalVariableRule(test_support.TestBase):
    def setUp(self):
        self.set_rules_selection(['UninitializedLocalVariableRule'])

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
                    'id': 'uninitialized local variable',
                    'severity': 'error',
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
                    'id': 'uninitialized local variable',
                    'severity': 'error',
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

if __name__ == '__main__':
    test_support.main()