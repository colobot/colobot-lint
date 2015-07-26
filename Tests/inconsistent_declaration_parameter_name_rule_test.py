#!/usr/bin/env python3
import test_support

class TestInconsistentDeclarationParameterNameRule(test_support.TestBase):
    def setUp(self):
        self.set_rules_selection(['InconsistentDeclarationParameterNameRule'])

    def test_only_function_declaration(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo(int a, int b, int c);'
            ],
            expected_errors = [])

    def test_only_function_definition(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo(int a, int b, int c) {}'
            ],
            expected_errors = [])

    def test_function_declaration_with_empty_parameter_names_and_function_definition(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo(int, int, int);',
                'void Foo(int a, int b, int c) {}'
            ],
            expected_errors = [])

    def test_function_declaration_and_function_definition_with_empty_parameter_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo(int a, int b, int c);',
                'void Foo(int, int, int) {}'
            ],
            expected_errors = [])

    def test_function_declaration_and_function_definition_with_inconsistent_parameter_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo(int a, int b, int c);',
                'void Foo(int x, int y, int z) {}'
            ],
            expected_errors = [
                {
                    'id': 'inconsistent declaration parameter name',
                    'severity': 'style',
                    'msg': "Function 'Foo' has other declaration(s) with inconsistently named parameter(s)",
                    'line': '1'
                }
            ])

    def test_multiple_function_declarations_and_function_definition_with_inconsistent_parameter_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo(int a, int b, int c);',
                'void Foo(int d, int e, int f);',
                'void Foo(int x, int y, int z) {}'
            ],
            expected_errors = [
                {
                    'id': 'inconsistent declaration parameter name',
                    'severity': 'style',
                    'msg': "Function 'Foo' has other declaration(s) with inconsistently named parameter(s)",
                    'line': '1'
                }
            ])

if __name__ == '__main__':
    test_support.main()