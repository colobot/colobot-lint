import test_support

class UninitializedFieldRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['UninitializedFieldRule'])
        self.set_default_error_id('uninitialized field')
        self.set_default_error_severity('error')

    def test_struct_with_field_initialization_in_declaration(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    int x = 0;',
                '    float y = 0.0f;',
                '    bool z = true;',
                '};'
            ],
            expected_errors = [])

    def test_struct_with_one_field_uninitialized_in_declaration(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    int x = 0;',
                '    float y;',
                '    bool z = true;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Struct 'Foo' field 'y' remains uninitialized",
                    'line': '4'
                }
            ])

    def test_struct_with_constructor_initialization_list(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo() : x(0), y(0.0f), z(true) {}',
                '    int x;',
                '    float y;',
                '    bool z;',
                '};'
            ],
            expected_errors = [])

    def test_struct_with_two_fields_uninitialized_in_constructor_initialization_list(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo() : y(0.0f), z(true) {}',
                '    int x;',
                '    float y;',
                '    bool z;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Struct 'Foo' field 'x' remains uninitialized in constructor",
                    'line': '3'
                }
            ])

    def test_struct_with_fields_initialized_inside_constructor(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo() { x = 0; y = 0.0f; z = true; }',
                '    int x;',
                '    float y;',
                '    bool z;',
                '};'
            ],
            expected_errors = [])

    def test_struct_with_fields_with_default_constructors(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Bar',
                '{',
                '   int x;',
                '   Bar() : x(0) {}',
                '};',
                'struct Baz',
                '{',
                '   int x = 0;',
                '};',
                'struct Foo',
                '{',
                '    Bar x;',
                '    Baz y;',
                '};'
            ],
            expected_errors = [])

    def test_struct_with_field_without_default_constructor(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Bar',
                '{',
                '   int x;',
                '};',
                'struct Foo',
                '{',
                '    Bar z;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Struct 'Bar' field 'x' remains uninitialized",
                    'line': '3'
                },
                {
                    'msg': "Struct 'Foo' field 'z' remains uninitialized",
                    'line': '7'
                }
            ])

    def test_class_with_multiple_constructors_one_constructor_without_one_field_assignment(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo() : z(true) {}',
                '    Foo(int xx) { x = xx; z = false; }',
                '    Foo(float yy) : y(yy) {}',
                '    int x = 0;',
                '    float y = 0.0f;',
                '    bool z;',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Struct 'Foo' field 'z' remains uninitialized in constructor",
                    'line': '5'
                }
            ])

    def test_class_with_constructor_declared_and_defined_elsewhere(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo();',
                '    int x = 0;',
                '    float y = 0.0f;',
                '    bool z;',
                '};',
                '',
                'Foo::Foo() : z(true) {}'
            ],
            expected_errors = [])

    def test_class_with_constructor_declared_and_defined_elsewhere_one_field_uninitialized(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo();',
                '    int x;',
                '    float y;',
                '    bool z;',
                '};',
                '',
                'Foo::Foo()',
                '{',
                '    x = 0;',
                '    y = 0;',
                '}'
            ],
            expected_errors = [
                {
                    'msg': "Struct 'Foo' field 'z' remains uninitialized in constructor",
                    'line': '9'
                }
            ])

    def test_ignore_union(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'union Foo',
                '{',
                '    int anything;',
                '    float anythingAtAll;',
                '};'
            ],
            expected_errors = [])

    def test_ignore_anonymous_union_but_not_struct(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    struct',
                '    {',
                '        int x;',
                '    };',
                '    union',
                '    {',
                '        int anything;',
                '        float anythingAtAll;',
                '    };',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Struct '' field 'x' remains uninitialized",
                    'line': '5'
                }
            ])

    def test_ignore_pod_types_with_no_data_members(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct PodWithoutData {};',
                'struct DerivedPodWithoutData : PodWithoutData {};',
                '',
                'struct Foo',
                '{',
                '    PodWithoutData pod1;',
                '    DerivedPodWithoutData pod2;',
                '};'
            ],
            expected_errors = [])

    def test_ignore_deleted_constructors(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '    Foo() : x(0), y(0.0f), z(false) {}',
                '    Foo(const Foo&) = delete;',
                '    int x;',
                '    float y;',
                '    bool z;',
                '};'
            ],
            expected_errors = [])

    def test_constructor_declared_but_not_defined_in_fake_header_source(self):
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                'foo/bar.h' : [
                    'class NoConstructor',
                    '{',
                    '   int x;',
                    '};',
                    'class ConstructorDefined',
                    '{',
                    '   ConstructorDefined() {}',
                    '   int x;',
                    '};',
                    'class NotDefinedConstructor',
                    '{',
                    '   NotDefinedConstructor();',
                    '   int x;',
                    '};'
                ],
                'fake_header_sources/foo/bar.cpp': [
                    '#include "foo/bar.h"'
                ]
            },
            compilation_database_files = ['fake_header_sources/foo/bar.cpp'],
            target_files = ['fake_header_sources/foo/bar.cpp'],
            additional_compile_flags = ['-I$TEMP_DIR'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR'],
            expected_errors = [
                {
                    'msg': "Class 'NoConstructor' field 'x' remains uninitialized",
                    'line': '3'
                },
                {
                    'msg': "Class 'ConstructorDefined' field 'x' remains uninitialized in constructor",
                    'line': '7'
                }
            ])

    def test_struct_declaration_in_header_constructor_definition_in_cpp_module(self):
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                'src.h' : [
                    'struct Foo',
                    '{',
                    '    Foo();',
                    '    int x;',
                    '    float y;',
                    '    bool z;',
                    '};',
                ],
                'src.cpp': [
                    '#include "src.h"',
                    '',
                    'Foo::Foo()',
                    '{',
                    '    x = 0;',
                    '    y = 0;',
                    '}'
                ]
            },
            compilation_database_files = ['src.cpp'],
            target_files = ['src.cpp'],
            expected_errors = [
                {
                    'msg': "Struct 'Foo' field 'z' remains uninitialized in constructor",
                    'line': '3'
                }
            ])