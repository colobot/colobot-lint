import test_support

class VariableNamingRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['VariableNamingRule'])
        self.set_default_error_id('variable naming')
        self.set_default_error_severity('style')

    def test_function_local_variable_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo { int x; };',
                'void Bar()',
                '{',
                '   int camel = 1;',
                '   float camelCase = 2.0f;',
                '   const char* camelCaseName = "3";',
                '   Foo aDDoubledCapitalInCamelCaseName{4};',
                '}'
            ],
            expected_errors = [])

    def test_function_local_variable_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Bar()',
                '{',
                '   int under_score_name = 1;',
                '   float CapitalCaseName = 2.0f;',
                '   const char* ALLCAPS = "3";',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Local variable 'under_score_name' should be named in camelCase style",
                    'line': '3'
                },
                {
                    'msg': "Local variable 'CapitalCaseName' should be named in camelCase style",
                    'line': '4'
                },
                {
                    'msg': "Local variable 'ALLCAPS' should be named in camelCase style",
                    'line': '5'
                }
            ])

    def test_function_parameter_variable_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo { int x; };',
                'void Bar(int camel,',
                '         float camelCase,',
                '         const char* camelCaseName,',
                '         Foo aDDoubledCapitalInCamelCaseName)',
                '{',
                '}'
            ],
            expected_errors = [])

    def test_function_parameter_variable_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Bar(int under_score_name,',
                '         float CapitalCaseName,',
                '         const char* ALLCAPS)',
                '{',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Local variable 'under_score_name' should be named in camelCase style",
                    'line': '1'
                },
                {
                    'msg': "Local variable 'CapitalCaseName' should be named in camelCase style",
                    'line': '2'
                },
                {
                    'msg': "Local variable 'ALLCAPS' should be named in camelCase style",
                    'line': '3'
                }
            ])

    def test_const_global_variable_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo { int x; };',
                'const int ALL = 1;',
                'const float ALL_CAPITAL = 2.0f;',
                'const char* const ALL_CAPITAL_LETTERS = "3";',
                'const Foo ALL_CAPITALS{4};'
            ],
            expected_errors = [])

    def test_const_global_variable_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo { int x; };',
                'const int NOT_ALL_capitals = 1;',
                'const float _STARTING_FROM_UNDERSCORE = 2.0f;',
                'const char* const camelCase = "3";',
                'const Foo g_namedLikeNonConstGlobal{4};'
            ],
            expected_errors = [
                {
                    'msg': "Const global variable 'NOT_ALL_capitals' should be named in ALL_CAPS style",
                    'line': '2'
                },
                {
                    'msg': "Const global variable '_STARTING_FROM_UNDERSCORE' should be named in ALL_CAPS style",
                    'line': '3'
                },
                {
                    'msg': "Const global variable 'camelCase' should be named in ALL_CAPS style",
                    'line': '4'
                },
                {
                    'msg': "Const global variable 'g_namedLikeNonConstGlobal' should be named in ALL_CAPS style",
                    'line': '5'
                }
            ])

    def test_non_const_global_variable_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo { int x; };',
                'int g_prefixed = 1;',
                'float g_prefixedCamel = 2.0f;',
                'const char* g_prefixedCamelCase = "3";',
                'Foo g_prefixedCamelCaseName{4};'
            ],
            expected_errors = [])

    def test_non_const_global_variable_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo { int x; };',
                'int notPrefixedCamelCase = 1;',
                'float m_wronglyPefixed = 2.0f;',
                'const char* ALL_CAPS = "3";',
                'Foo CapitalCaseName{4};'
            ],
            expected_errors = [
                {
                    'msg': "Non-const global variable 'notPrefixedCamelCase' should be named in g_camelCase style",
                    'line': '2'
                },
                {
                    'msg': "Non-const global variable 'm_wronglyPefixed' should be named in g_camelCase style",
                    'line': '3'
                },
                {
                    'msg': "Non-const global variable 'ALL_CAPS' should be named in g_camelCase style",
                    'line': '4'
                },
                {
                    'msg': "Non-const global variable 'CapitalCaseName' should be named in g_camelCase style",
                    'line': '5'
                }
            ])

    def test_deprecated_variable_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Bar()',
                '{',
                '  bool bBool = false;',
                '  int* pPtr = nullptr;',
                '  int p1 = 0; // but this is fine',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Local variable 'bBool' is named in a style that is deprecated",
                    'line': '3'
                },
                {
                    'msg': "Local variable 'pPtr' is named in a style that is deprecated",
                    'line': '4'
                }
            ])

    def test_public_static_class_member_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '   static const int camelCase;',
                '   static float camelCaseName;',
                '   static const char* aDDoubledCapitalInCamelCaseName;',
                '};'
            ],
            expected_errors = [])

    def test_public_static_class_member_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '   static const int under_score_name;',
                '   static float CapitalCaseName;',
                '   static const char* ALLCAPS;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Public field 'under_score_name' should be named in camelCase style",
                    'line': '3'
                },
                {
                    'msg': "Public field 'CapitalCaseName' should be named in camelCase style",
                    'line': '4'
                },
                {
                    'msg': "Public field 'ALLCAPS' should be named in camelCase style",
                    'line': '5'
                }
            ])

    def test_private_and_protected_static_class_member_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{',
                '   static const int m_camelCase;',
                'protected:',
                '   static float m_camelCaseName;',
                'private:',
                '   static const char* m_aDDoubledCapitalInCamelCaseName;',
                '};'
            ],
            expected_errors = [])

    def test_private_and_protected_static_class_member_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{',
                '   static int under_score_name;',
                'protected:',
                '   static float CapitalCaseName;',
                'private:',
                '   static const char* ALLCAPS;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Private field 'under_score_name' should be named in m_camelCase style",
                    'line': '3'
                },
                {
                    'msg': "Protected field 'CapitalCaseName' should be named in m_camelCase style",
                    'line': '5'
                },
                {
                    'msg': "Private field 'ALLCAPS' should be named in m_camelCase style",
                    'line': '7'
                }
            ])

    def test_public_class_member_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '   const int camelCase;',
                '   float camelCaseName;',
                '   const char* aDDoubledCapitalInCamelCaseName;',
                '};'
            ],
            expected_errors = [])

    def test_public_class_member_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '   const int under_score_name;',
                '   float CapitalCaseName;',
                '   const char* ALLCAPS;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Public field 'under_score_name' should be named in camelCase style",
                    'line': '3'
                },
                {
                    'msg': "Public field 'CapitalCaseName' should be named in camelCase style",
                    'line': '4'
                },
                {
                    'msg': "Public field 'ALLCAPS' should be named in camelCase style",
                    'line': '5'
                }
            ])

    def test_private_and_protected_class_member_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{',
                '   Foo();',
                '   const int m_camelCase;',
                'protected:',
                '   float m_camelCaseName;',
                'private:',
                '   const char* m_aDDoubledCapitalInCamelCaseName;',
                '};'
            ],
            expected_errors = [])

    def test_private_and_protected_class_member_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{',
                '   int under_score_name;',
                'protected:',
                '   float CapitalCaseName;',
                'private:',
                '   const char* ALLCAPS;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Private field 'under_score_name' should be named in m_camelCase style",
                    'line': '3'
                },
                {
                    'msg': "Protected field 'CapitalCaseName' should be named in m_camelCase style",
                    'line': '5'
                },
                {
                    'msg': "Private field 'ALLCAPS' should be named in m_camelCase style",
                    'line': '7'
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
                    'msg': "Private field 'm_bBool' is named in a style that is deprecated",
                    'line': '3'
                },
                {
                    'msg': "Private field 'm_pPtr' is named in a style that is deprecated",
                    'line': '4'
                }
            ])

    def test_ignore_compiler_generated_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Print(int);',
                'void PrintNums()',
                '{',
                '   int nums[] = {1, 2, 3, 4};',
                '   for (auto x : nums) // compiler generates __range, __begin and __end locals here',
                '   {',
                '       Print(x);',
                '   }',
                '',
                '}'
            ],
            expected_errors = [])

    def test_allow_names_with_numbers(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo { int x; };',
                'void Bar()',
                '{',
                '   int camel1 = 1;',
                '   float camelCase2 = 2.0f;',
                '   const char* camel123CaseName = "3";',
                '   Foo aDoubled44NumberInCamelCaseName{4};',
                '}'
            ],
            expected_errors = [])
