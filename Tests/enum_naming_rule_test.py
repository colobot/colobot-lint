#!/usr/bin/env python3
import test_support

class EnumNamingRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['EnumNamingRule'])
        self.set_default_error_id('enum naming')
        self.set_default_error_severity('style')

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
                    'msg': "Enum class 'lower' should be named in a style like UpperCamelCase",
                    'line': '1'
                },
                {
                    'msg': "Enum class 'lowerCase' should be named in a style like UpperCamelCase",
                    'line': '2'
                },
                {
                    'msg': "Enum class 'lower_case' should be named in a style like UpperCamelCase",
                    'line': '3'
                },
                {
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
                    'msg': "Enum class constant 'lower' should be named in a style like UpperCamelCase",
                    'line': '3'
                },
                {
                    'msg': "Enum class constant 'lowerCase' should be named in a style like UpperCamelCase",
                    'line': '4'
                },
                {
                    'msg': "Enum class constant 'lower_case' should be named in a style like UpperCamelCase",
                    'line': '5'
                },
                {
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
                    'severity': 'information',
                    'msg': "Anonymous enum",
                    'line': '3'
                }
            ])
