#!/usr/bin/env python3
import test_support

class TestFunctionNamingRule(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['FunctionNamingRule'])
        self.set_default_error_id('function naming')
        self.set_default_error_severity('style')

    def test_function_declaration_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Camel();',
                'void CamelCase();',
                'void Camel2CaseName();'
            ],
            expected_errors = [])

    def test_function_declaration_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void under_score_name();',
                'void lowerCamelCaseName();',
                'void ALLCAPS_NAME();'
            ],
            expected_errors = [
                {
                    'msg': "Function 'under_score_name' should be named in UpperCamelCase style",
                    'line': '1'
                },
                {
                    'msg': "Function 'lowerCamelCaseName' should be named in UpperCamelCase style",
                    'line': '2'
                },
                {
                    'msg': "Function 'ALLCAPS_NAME' should be named in UpperCamelCase style",
                    'line': '3'
                }
            ])

    def test_function_definition_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Camel() {}',
                'void CamelCase() {}',
                'void Camel2CaseName() {}'
            ],
            expected_errors = [])

    def test_function_definition_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void under_score_name() {}',
                'void lowerCamelCaseName() {}',
                'void ALLCAPS_NAME() {}'
            ],
            expected_errors = [
                {
                    'line': '1',
                    'msg': "Function 'under_score_name' should be named in UpperCamelCase style",
                },
                {
                    'msg': "Function 'lowerCamelCaseName' should be named in UpperCamelCase style",
                    'line': '2'
                },
                {
                    'msg': "Function 'ALLCAPS_NAME' should be named in UpperCamelCase style",
                    'line': '3'
                }
            ])

    def test_function_definition_and_declaration_incorrect_names_dont_report_double_errors(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void under_score_name();',
                'void under_score_name() {}',
                'void lowerCamelCaseName();',
                'void lowerCamelCaseName() {}',
                'void ALLCAPS_NAME();',
                'void ALLCAPS_NAME() {}'
            ],
            expected_errors = [
                {
                    'line': '1',
                    'msg': "Function 'under_score_name' should be named in UpperCamelCase style",
                },
                {
                    'msg': "Function 'lowerCamelCaseName' should be named in UpperCamelCase style",
                    'line': '3'
                },
                {
                    'msg': "Function 'ALLCAPS_NAME' should be named in UpperCamelCase style",
                    'line': '5'
                }
            ])

    def test_function_declaration_in_namespace_and_outside_namespace_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'namespace Foo',
                '{',
                'void under_score_name();',
                '}',
                'void under_score_name();'
            ],
            expected_errors = [
                {
                    'line': '3',
                    'msg': "Function 'under_score_name' should be named in UpperCamelCase style",
                },
                {
                    'line': '5',
                    'msg': "Function 'under_score_name' should be named in UpperCamelCase style",
                }
            ])

    def test_method_declaration_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{',
                '    void Camel();',
                '    void CamelCase();',
                '    void Camel2CaseName();',
                '};'
            ],
            expected_errors = [])

    def test_method_declaration_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{',
                '    void under_score_name();',
                '    void lowerCamelCaseName();',
                '    void ALLCAPS_NAME();',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Method 'under_score_name' should be named in UpperCamelCase style",
                    'line': '3'
                },
                {
                    'msg': "Method 'lowerCamelCaseName' should be named in UpperCamelCase style",
                    'line': '4'
                },
                {
                    'msg': "Method 'ALLCAPS_NAME' should be named in UpperCamelCase style",
                    'line': '5'
                }
            ])

    def test_method_definition_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{',
                '    void Camel() {}',
                '    void CamelCase() {}',
                '    void Camel2CaseName() {}',
                '};'
            ],
            expected_errors = [])

    def test_method_definition_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{',
                '    void under_score_name() {}',
                '    void lowerCamelCaseName() {}',
                '    void ALLCAPS_NAME() {}',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Method 'under_score_name' should be named in UpperCamelCase style",
                    'line': '3'
                },
                {
                    'msg': "Method 'lowerCamelCaseName' should be named in UpperCamelCase style",
                    'line': '4'
                },
                {
                    'msg': "Method 'ALLCAPS_NAME' should be named in UpperCamelCase style",
                    'line': '5'
                }
            ])

    def test_method_declaration_and_definition_incorrect_names_dont_report_double_errors(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{',
                '    void under_score_name();',
                '    void lowerCamelCaseName();',
                '    void ALLCAPS_NAME();',
                '};',
                'void Foo::under_score_name() {}',
                'void Foo::lowerCamelCaseName() {}',
                'void Foo::ALLCAPS_NAME() {}',
            ],
            expected_errors = [
                {
                    'msg': "Method 'under_score_name' should be named in UpperCamelCase style",
                    'line': '3'
                },
                {
                    'msg': "Method 'lowerCamelCaseName' should be named in UpperCamelCase style",
                    'line': '4'
                },
                {
                    'msg': "Method 'ALLCAPS_NAME' should be named in UpperCamelCase style",
                    'line': '5'
                }
            ])

    def test_virtual_overridden_method_incorrect_name_report_only_base_class_name(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Base',
                '{',
                '    virtual ~Base() {}',
                '    virtual void under_score_name() {}',
                '};',
                'struct Derived : public Base',
                '{',
                '    void under_score_name() override {}',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Method 'under_score_name' should be named in UpperCamelCase style",
                    'line': '4'
                }
            ])

    def test_operator_overloads_are_skipped(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo& operator=(const Foo& other);',
                '};',
                'Foo operator+(const Foo& a, const Foo& b);'
            ],
            expected_errors = [])

    def test_constructors_destructors_and_conversion_operators_are_skipped(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct under_score_name',
                '{',
                '    under_score_name();',
                '    ~under_score_name();',
                '    operator bool();',
                '};'
            ],
            expected_errors = [])

    def test_iterator_access_functions_are_an_exception(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    int begin();',
                '    int end();',
                '};'
            ],
            expected_errors = [])

    def test_repeating_uppercase_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void RRepeatingUpperCaseLetterName();',
                'void LaterRRepeatingUpperCaseLetterName();'
            ],
            expected_errors = [])

if __name__ == '__main__':
    test_support.main()