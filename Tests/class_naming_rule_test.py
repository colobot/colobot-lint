#!/usr/bin/env python3
import test_support

class ClassNamingRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['ClassNamingRule'])
        self.set_default_error_id('class naming')
        self.set_default_error_severity('style')

    def test_correct_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class CFoo { int x; float y; };',
                'struct Bar { int x; float y; };',
                'union Baz { int x; float y; };'
            ],
            expected_errors = [])

    def test_incorrect_class_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Cfoo { int x; float y; };',
                'class Foo { int x; float y; };',
                'class lowerCase { int x; float y; };',
                'class lower_case { int x; float y; };',
                'class ALL_CAPS { int x; float y; };',
            ],
            expected_errors = [
                {
                    'msg': "Class 'Cfoo' should be named in a style like CUpperCamelCase",
                    'line': '1'
                },
                {
                    'msg': "Class 'Foo' should be named in a style like CUpperCamelCase",
                    'line': '2'
                },
                {
                    'msg': "Class 'lowerCase' should be named in a style like CUpperCamelCase",
                    'line': '3'
                },
                {
                    'msg': "Class 'lower_case' should be named in a style like CUpperCamelCase",
                    'line': '4'
                },
                {
                    'msg': "Class 'ALL_CAPS' should be named in a style like CUpperCamelCase",
                    'line': '5'
                }
            ])

    def test_incorrect_struct_and_union_names(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct CFoo { int x; float y; };',
                'struct foo { int x; float y; };',
                'union lowerCase { int x; float y; };',
                'union lower_case { int x; float y; };',
                'struct ALL_CAPS { int x; float y; };',
            ],
            expected_errors = [
                {
                    'msg': "Struct 'CFoo' follows class naming style CUpperCamelCase but is not a class",
                    'line': '1'
                },
                {
                    'msg': "Struct 'foo' should be named in a style like UpperCamelCase",
                    'line': '2'
                },
                {
                    'msg': "Union 'lowerCase' should be named in a style like UpperCamelCase",
                    'line': '3'
                },
                {
                    'msg': "Union 'lower_case' should be named in a style like UpperCamelCase",
                    'line': '4'
                },
                {
                    'msg': "Struct 'ALL_CAPS' should be named in a style like UpperCamelCase",
                    'line': '5'
                }
            ])

    def test_inform_about_anonymous_structs(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '   struct',
                '   {',
                '      int x;',
                '      float y;',
                '   } foo;',
                '};'
            ],
            expected_errors = [
                {
                    'severity': 'information',
                    'msg': 'Anonymous struct',
                    'line': '3'
                }
            ])

    def test_ignore_lambdas(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'int main()',
                '{',
                '   int x = 1;',
                '   int y = 2;',
                '   auto foo = [x,&y]() -> int',
                '   {',
                '      return x + y;',
                '   };',
                '}'
            ],
            expected_errors = [])

if __name__ == '__main__':
    test_support.main()
