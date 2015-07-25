#!/usr/bin/env python3
import test_support

class TestVariableNamingRule(test_support.TestBase):
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
                '  int p1 = 0; // but this is fine',
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

    def test_public_static_class_member_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Foo',
                '{',
                '   static const int camelCase;',
                '   static float camelCaseName;',
                '   static std::string aDDoubledCapitalInCamelCaseName;',
                '}'
            ],
            expected_errors = [])

    def test_public_static_class_member_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Foo',
                '{',
                '   static const int under_score_name;',
                '   static float CapitalCaseName;',
                '   static std::string ALLCAPS;',
                '}'
            ],
            expected_errors = [
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Public field 'under_score_name' should be named in camelCase style",
                    'line': '4'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Public field 'CapitalCaseName' should be named in camelCase style",
                    'line': '5'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Public field 'ALLCAPS' should be named in camelCase style",
                    'line': '6'
                }
            ])

    def test_private_and_protected_static_class_member_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'class Foo',
                '{',
                '   static const int m_camelCase;',
                'protected:',
                '   static float m_camelCaseName;',
                'private:',
                '   static std::string m_aDDoubledCapitalInCamelCaseName;',
                '}'
            ],
            expected_errors = [])

    def test_private_and_protected_static_class_member_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'class Foo',
                '{',
                '   static int under_score_name;',
                'protected:',
                '   static float CapitalCaseName;',
                'private:',
                '   static std::string ALLCAPS;',
                '}'
            ],
            expected_errors = [
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Private field 'under_score_name' should be named in m_camelCase style",
                    'line': '4'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Protected field 'CapitalCaseName' should be named in m_camelCase style",
                    'line': '6'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Private field 'ALLCAPS' should be named in m_camelCase style",
                    'line': '8'
                }
            ])

    def test_public_class_member_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Foo',
                '{',
                '   const int camelCase;',
                '   float camelCaseName;',
                '   std::string aDDoubledCapitalInCamelCaseName;',
                '}'
            ],
            expected_errors = [])

    def test_public_class_member_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Foo',
                '{',
                '   const int under_score_name;',
                '   float CapitalCaseName;',
                '   std::string ALLCAPS;',
                '}'
            ],
            expected_errors = [
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Public field 'under_score_name' should be named in camelCase style",
                    'line': '4'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Public field 'CapitalCaseName' should be named in camelCase style",
                    'line': '5'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Public field 'ALLCAPS' should be named in camelCase style",
                    'line': '6'
                }
            ])

    def test_private_and_protected_class_member_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'class Foo',
                '{',
                '   const int m_camelCase;',
                'protected:',
                '   float m_camelCaseName;',
                'private:',
                '   std::string m_aDDoubledCapitalInCamelCaseName;',
                '}'
            ],
            expected_errors = [])

    def test_private_and_protected_class_member_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'class Foo',
                '{',
                '   int under_score_name;',
                'protected:',
                '   float CapitalCaseName;',
                'private:',
                '   std::string ALLCAPS;',
                '}'
            ],
            expected_errors = [
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Private field 'under_score_name' should be named in m_camelCase style",
                    'line': '4'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Protected field 'CapitalCaseName' should be named in m_camelCase style",
                    'line': '6'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Private field 'ALLCAPS' should be named in m_camelCase style",
                    'line': '8'
                }
            ])

    def test_deprecated_class_member_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{',
                '  bool m_bBool;',
                '  int* m_pPtr;',
                '  int m_p1; // but this is fine',
                '};'
            ],
            expected_errors = [
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Private field 'm_bBool' is named in a style that is deprecated",
                    'line': '3'
                },
                {
                    'id': 'variable naming',
                    'severity': 'style',
                    'msg': "Private field 'm_pPtr' is named in a style that is deprecated",
                    'line': '4'
                }
            ])

    def test_ignore_compiler_generated_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <vector>',
                '#include <iostream>',
                'void printNums(const std::vector<int>& vec)',
                '{',
                '   for (auto x : vec) // compiler generates __range, __begin and __end locals here',
                '   {',
                '       std::cout << x << std::endl;',
                '   }',
                '',
                '}'
            ],
            expected_errors = [])

    def test_allow_names_with_numbers(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Foo { int x; };',
                'int main()',
                '{',
                '   int camel1 = 1;',
                '   float camelCase2 = 2.0f;',
                '   std::string camel123CaseName = "3";',
                '   Foo aDoubled44NumberInCamelCaseName{4};',
                '}'
            ],
            expected_errors = [])

if __name__ == '__main__':
    test_support.main()