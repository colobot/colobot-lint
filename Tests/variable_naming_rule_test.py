#!/usr/bin/env python3
import unittest
import test_support
import sys

class TestBlockPlacementRule(test_support.TestBase):
    def setUp(self):
        self.set_rules_selection(['VariableNamingRule'])

    def test_function_local_variable_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Foo { int x; };',
                'int main()',
                '{',
                '   int camel = 1;',
                '   float camelCase = 2.0f;',
                '   std::string camelCaseName = "3";',
                '   Foo aDDoubledCapitalInCamelCaseName{4};',
                '}'
            ],
            expected_errors = [])

    def test_function_local_variable_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'int main()',
                '{',
                '   int under_score_name = 1;',
                '   float CapitalCaseName = 2.0f;',
                '   std::string ALLCAPS = "3";',
                '}'
            ],
            expected_errors = [
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Local variable 'under_score_name' should be named in camelCase style",
                    'line': '4'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Local variable 'CapitalCaseName' should be named in camelCase style",
                    'line': '5'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Local variable 'ALLCAPS' should be named in camelCase style",
                    'line': '6'
                }
            ])

    def test_function_parameter_variable_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Foo { int x; };',
                'int main(int camel,',
                '         float camelCase,',
                '         std::string camelCaseName,',
                '         Foo aDDoubledCapitalInCamelCaseName)',
                '{',
                '}'
            ],
            expected_errors = [])

    def test_function_parameter_variable_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'int main(int under_score_name,',
                '         float CapitalCaseName,',
                '         std::string ALLCAPS)',
                '{',
                '}'
            ],
            expected_errors = [
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Local variable 'under_score_name' should be named in camelCase style",
                    'line': '2'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Local variable 'CapitalCaseName' should be named in camelCase style",
                    'line': '3'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Local variable 'ALLCAPS' should be named in camelCase style",
                    'line': '4'
                }
            ])

    def test_const_global_variable_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Foo { int x; };',
                'const int ALL = 1;',
                'const float ALL_CAPITAL = 2.0f;',
                'const std::string ALL_CAPITAL_LETTERS = "3";',
                'const Foo ALL_CAPITALS{4};'
            ],
            expected_errors = [])

    def test_const_global_variable_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Foo { int x; };',
                'const int NOT_ALL_capitals = 1;',
                'const float _STARTING_FROM_UNDERSCORE = 2.0f;',
                'const std::string camelCase = "3";',
                'const Foo g_namedLikeNonConstGlobal{4};'
            ],
            expected_errors = [
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Const global variable 'NOT_ALL_capitals' should be named in ALL_CAPS style",
                    'line': '3'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Const global variable '_STARTING_FROM_UNDERSCORE' should be named in ALL_CAPS style",
                    'line': '4'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Const global variable 'camelCase' should be named in ALL_CAPS style",
                    'line': '5'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Const global variable 'g_namedLikeNonConstGlobal' should be named in ALL_CAPS style",
                    'line': '6'
                }
            ])

    def test_non_const_global_variable_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Foo { int x; };',
                'int g_prefixed = 1;',
                'float g_prefixedCamel = 2.0f;',
                'std::string g_prefixedCamelCase = "3";',
                'Foo g_prefixedCamelCaseName{4};'
            ],
            expected_errors = [])

    def test_non_const_global_variable_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Foo { int x; };',
                'int notPrefixedCamelCase = 1;',
                'float m_wronglyPefixed = 2.0f;',
                'std::string ALL_CAPS = "3";',
                'Foo CapitalCaseName{4};'
            ],
            expected_errors = [
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Non-const global variable 'notPrefixedCamelCase' should be named in g_camelCase style",
                    'line': '3'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Non-const global variable 'm_wronglyPefixed' should be named in g_camelCase style",
                    'line': '4'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Non-const global variable 'ALL_CAPS' should be named in g_camelCase style",
                    'line': '5'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Non-const global variable 'CapitalCaseName' should be named in g_camelCase style",
                    'line': '6'
                }
            ])

    def test_deprecated_variable_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'int main()',
                '{',
                '  bool bBool = false;',
                '  int* pPtr = nullptr;',
                '}'
            ],
            expected_errors = [
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Local variable 'bBool' is named in a style that is deprecated",
                    'line': '3'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Local variable 'pPtr' is named in a style that is deprecated",
                    'line': '4'
                }
            ])

if __name__ == '__main__':
    if len(sys.argv) >= 2:
        test_support.colobot_lint_exectuable = sys.argv.pop(1)

    unittest.main()