#!/usr/bin/env python3
import test_support

class InconsistentDeclarationParameterNameRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['InconsistentDeclarationParameterNameRule'])
        self.set_default_error_id('inconsistent declaration parameter name')
        self.set_default_error_severity('style')

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
                    'msg': "Function 'Foo' has other declaration(s) with inconsistently named parameter(s)",
                    'line': '1'
                }
            ])

    def test_class_method_declaration_and_definition(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '   void Bar(int a, int b, int c);',
                '};'
                'void Foo::Bar(int a, int b, int c) {}',
            ],
            expected_errors = [])

    def test_class_method_declaration_and_definition_inconsistent_parameter_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '   void Bar(int a, int b, int c);',
                '};'
                'void Foo::Bar(int x, int y, int z) {}',
            ],
            expected_errors = [
                {
                    'msg': "Function 'Foo::Bar' has other declaration(s) with inconsistently named parameter(s)",
                    'line': '3'
                }
            ])

if __name__ == '__main__':
    test_support.main()