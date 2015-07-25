#!/usr/bin/env python3
import test_support

class TestEnumNamingRule(test_support.TestBase):
    def setUp(self):
        self.set_rules_selection(['EnumNamingRule'])

    def test_correct_enum_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'enum class Upper { Const };',
                'enum class UpperCamel { Const };',
                'enum class UpperCamelCase { Const };'
            ],
            expected_errors = [])

    def test_incorrect_enum_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'enum class lower { Const };',
                'enum class lowerCase { Const };',
                'enum class lower_case { Const };',
                'enum class ALL_CAPS { Const };',
            ],
            expected_errors = [
                {
                    'id': 'enum naming',
                    'severity': 'style',
                    'msg': "Enum class 'lower' should be named in a style like UpperCamelCase",
                    'line': '1'
                },
                {
                    'id': 'enum naming',
                    'severity': 'style',
                    'msg': "Enum class 'lowerCase' should be named in a style like UpperCamelCase",
                    'line': '2'
                },
                {
                    'id': 'enum naming',
                    'severity': 'style',
                    'msg': "Enum class 'lower_case' should be named in a style like UpperCamelCase",
                    'line': '3'
                },
                {
                    'id': 'enum naming',
                    'severity': 'style',
                    'msg': "Enum class 'ALL_CAPS' should be named in a style like UpperCamelCase",
                    'line': '4'
                }
            ])

    def test_enum_constant_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'enum class Foo',
                '{',
                '   Upper,',
                '   UpperCamel,',
                '   UpperCamelCase',
                '};',
            ],
            expected_errors = [])

    def test_enum_constant_incorrect_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'enum class Foo',
                '{',
                '   lower,',
                '   lowerCase,',
                '   lower_case,',
                '   ALL_CAPS',
                '};'
            ],
            expected_errors = [
                {
                    'id': 'enum naming',
                    'severity': 'style',
                    'msg': "Enum class constant 'lower' should be named in a style like UpperCamelCase",
                    'line': '3'
                },
                {
                    'id': 'enum naming',
                    'severity': 'style',
                    'msg': "Enum class constant 'lowerCase' should be named in a style like UpperCamelCase",
                    'line': '4'
                },
                {
                    'id': 'enum naming',
                    'severity': 'style',
                    'msg': "Enum class constant 'lower_case' should be named in a style like UpperCamelCase",
                    'line': '5'
                },
                {
                    'id': 'enum naming',
                    'severity': 'style',
                    'msg': "Enum class constant 'ALL_CAPS' should be named in a style like UpperCamelCase",
                    'line': '6'
                }
            ])

    def test_old_style_enum(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'enum OldStyleEnum',
                '{',
                '   any,',
                '   anything,',
                '   anything_at_all',
                '};'
            ],
            expected_errors = [
                {
                    'id': 'enum naming',
                    'severity': 'information',
                    'msg': "Old-style enum 'OldStyleEnum'",
                    'line': '1'
                }
            ])

    def test_inform_about_anonymous_enum(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo',
                '{',
                '   enum',
                '   {',
                '      anything',
                '   };',
                '};'
            ],
            expected_errors = [
                {
                    'id': 'enum naming',
                    'severity': 'information',
                    'msg': "Anonymous enums are not allowed",
                    'line': '3'
                }
            ])

if __name__ == '__main__':
    test_support.main()
