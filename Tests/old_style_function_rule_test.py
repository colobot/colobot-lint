#!/usr/bin/env python3
import test_support

class OldStyleFunctionRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['OldStyleFunctionRule'])
        self.set_default_error_id('old style function')
        self.set_default_error_severity('warning')

    def test_declaration_and_point_of_use_in_next_line(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'int Bar() { return 42; }',
                'void Foo()',
                '{',
                '    int x;',
                '    x = Bar();',
                '}'
            ],
            expected_errors = [])

    def test_declaration_and_point_of_use_in_third_line_after(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Bar(int &x, int &y);',
                'void Foo()',
                '{',
                '    int x;',
                '    int y;',
                '    Bar(x, y);',
                '}'
            ],
            expected_errors = [])

    def test_declaration_and_point_of_use_in_fourth_line_after(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Bar(int &x, int &y, int& z);',
                'void Foo()',
                '{',
                '    int x;',
                '    int y;',
                '    int z;',
                '    Bar(x, y, z);',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Function 'Foo' seems to be written in legacy C style: " +
                            "it has uninitialized POD type variables declared far from their point of use ('x')",
                    'line': '2'
                }
            ])

    def test_multiple_old_style_declarations(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'void Bar(int &x, int &y, int& z);',
                'void Foo()',
                '{',
                '    int x, y, z;',
                '    float a, b, c',
                '    std::string str;',
                '',
                '    Bar(x, y, z);',
                '    a = b = c = 10.0f;',
                '    str = "123";',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Function 'Foo' seems to be written in legacy C style: " +
                            "it has uninitialized POD type variables declared far from their point of use ('x', 'y', 'z', 'a'... and 2 more)",
                    'line': '3'
                }
            ])

    def test_ignore_function_parameters(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Bar(int &x, int &y, int& z, int& w)',
                '{',
                '    x = 1;',
                '    y = 2;',
                '    z = 3;',
                '    w = 4;'
                '}'
            ],
            expected_errors = [])

    def test_ignore_initialized_declarations(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'void Bar(int &x, int &y, int& z);',
                'void Foo()',
                '{',
                '    int x = 0, y = 0, z = 0;',
                '    float a{}, b{}, c{}',
                '    std::string str;',
                '',
                '    Bar(x, y, z);',
                '    a = b = c = 10.0f;',
                '    str = "123";',
                '}'
            ],
            expected_errors = [])

    def test_ignore_non_pod_type_declarations(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include <string>',
                'struct Pod { int x; };',
                'struct NonPod { int x = 0; };',
                'void Foo()',
                '{',
                '    Pod pod;',
                '    NonPod nonPod;',
                '    std::string str;',
                '',
                '    pod = Pod{10};',
                '    nonPod = NonPod{20};',
                '    str = "123";',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Function 'Foo' seems to be written in legacy C style: " +
                            "it has uninitialized POD type variables declared far from their point of use ('pod')",
                    'line': '4'
                }
            ])


if __name__ == '__main__':
    test_support.main()
