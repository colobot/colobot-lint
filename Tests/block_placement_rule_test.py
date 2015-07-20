#!/usr/bin/env python3
import unittest
import test_support
import sys

class TestBlockPlacementRule(test_support.TestBase):
    # namespaces
    def test_namespace_braces_in_separate_lines(self):
        self.assert_colobot_lint_result(
            source_file_text = 'namespace test\n{\n}',
            expected_errors = [])

    def test_namespace_opening_and_closing_brace_in_same_line(self):
        self.assert_colobot_lint_result(
            source_file_text = 'namespace test { }',
            expected_errors = [])

    def test_namespace_opening_brace_in_same_line(self):
        self.assert_colobot_lint_result(
            source_file_text = 'namespace test {\n}',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Declaration has body that begins or ends in a style that is not allowed',
                    'line': '1'
                }
            ])

    def test_other_declaration_just_before_closing_namespace_brace(self):
        self.assert_colobot_lint_result(
            source_file_text = 'namespace test\n{\nvoid foo(); }',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Declaration has body that begins or ends in a style that is not allowed',
                    'line': '1'
                }
            ])

    def test_other_declaration_after_closing_namespace_brace(self):
        self.assert_colobot_lint_result(
            source_file_text = 'namespace test\n{\n} namespace foo { }',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Declaration has body that begins or ends in a style that is not allowed',
                    'line': '3'
                }
            ])

    # classes, enums, unions, etc.
    def test_union_braces_in_separate_lines(self):
        self.assert_colobot_lint_result(
            source_file_text = 'class test\n{\nint x;\nfloat y;\n};',
            expected_errors = [])

    def test_union_opening_and_closing_brace_in_same_line(self):
        self.assert_colobot_lint_result(
            source_file_text = 'class test { };',
            expected_errors = [])

    def test_enum_opening_brace_in_same_line(self):
        self.assert_colobot_lint_result(
            source_file_text = 'enum test {\nfoo\n};',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Declaration has body that begins or ends in a style that is not allowed',
                    'line': '1'
                }
            ])

    def test_other_declaration_just_before_closing_enum_brace(self):
        self.assert_colobot_lint_result(
            source_file_text = 'enum test\n{\n foo };',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Declaration has body that begins or ends in a style that is not allowed',
                    'line': '1'
                }
            ])

    def test_other_declaration_after_closing_enum_class_brace(self):
        self.assert_colobot_lint_result(
            source_file_text = 'enum class test\n{\nfoo\n}; namespace foo { }',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Declaration has body that begins or ends in a style that is not allowed',
                    'line': '4'
                }
            ])

    def test_nested_class_has_braces_in_invalid_style(self):
        self.assert_colobot_lint_result(
            source_file_text = 'class foo\n{\nclass bar {\n};\n};',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Declaration has body that begins or ends in a style that is not allowed',
                    'line': '3'
                }
            ])

    # functions
    def test_function_braces_in_separate_lines(self):
        self.assert_colobot_lint_result(
            source_file_text = 'void foo()\n{\n}',
            expected_errors = [])

    def test_function_opening_and_closing_brace_in_same_line(self):
        self.assert_colobot_lint_result(
            source_file_text = 'void foo()\n{ }',
            expected_errors = [])

    def test_function_opening_brace_in_same_line(self):
        self.assert_colobot_lint_result(
            source_file_text = 'void foo() {\nint x;\n}',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Statement has body that begins or ends in a style that is not allowed',
                    'line': '1'
                }
            ])

    def test_other_statement_just_before_closing_function_brace(self):
        self.assert_colobot_lint_result(
            source_file_text = 'void foo()\n{\nint x; }',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Statement has body that begins or ends in a style that is not allowed',
                    'line': '2'
                }
            ])

    def test_other_declaration_after_closing_function_brace(self):
        self.assert_colobot_lint_result(
            source_file_text = 'void test()\n{\nint x;\n} namespace foo { }',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Declaration has body that begins or ends in a style that is not allowed',
                    'line': '4'
                }
            ])

    def test_nested_statement_block_has_braces_in_invalid_style(self):
        self.assert_colobot_lint_result(
            source_file_text = 'void foo()\n{\nif (true) {\n}\n}',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Statement has body that begins or ends in a style that is not allowed',
                    'line': '3'
                }
            ])

    def test_multiline_function_argument_list_opening_brace_in_same_line(self):
        self.assert_colobot_lint_result(
            source_file_text = 'void foo(int a,\nint b) {\n}',
            expected_errors = [
                {
                    'id': 'code block placement',
                    'severity': 'style',
                    'msg': 'Statement has body that begins or ends in a style that is not allowed',
                    'line': '2'
                }
            ])

    def test_multiline_function_argument_list_opening_brace_in_new_line(self):
        self.assert_colobot_lint_result(
            source_file_text = 'void foo(int a,\nint b)\n{\n}',
            expected_errors = [])

if __name__ == '__main__':
    if len(sys.argv) >= 2:
        test_support.colobot_lint_exectuable = sys.argv.pop(1)

    unittest.main()