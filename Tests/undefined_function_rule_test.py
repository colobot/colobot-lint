import test_support

class UndefinedFunctionRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['UndefinedFunctionRule'])
        self.set_default_error_id('undefined function')
        self.set_default_error_severity('information')

    def test_function_defined(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo() {}'
            ],
            expected_errors = [])

    def test_function_forward_declared_then_defined(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo();',
                'void Foo() {}'
            ],
            expected_errors = [])

    def test_function_declared_but_not_defined(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo();'
            ],
            expected_errors = [
                {
                    'msg': "Function 'Foo' declared but never defined",
                    'line': '1'
                }
            ])

    def test_function_declared_in_header_and_then_defined_in_source_module(self):
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                'src.h' : [
                    'void Foo();'
                ],
                'fake_header_sources/src.cpp': [
                    '#include "src.h"'
                ],
                'src.cpp': [
                    '#include "src.h"',
                    'void Foo() {}'
                ]
            },
            compilation_database_files = ['fake_header_sources/src.cpp', 'src.cpp'],
            target_files = ['fake_header_sources/src.cpp', 'src.cpp'],
            additional_compile_flags = ['-I$TEMP_DIR'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR'],
            expected_errors = [])

    def test_function_declared_in_header_and_then_defined_in_source_module_reverse_processing_order(self):
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                'src.h' : [
                    'void Foo();'
                ],
                'fake_header_sources/src.cpp': [
                    '#include "src.h"'
                ],
                'src.cpp': [
                    '#include "src.h"',
                    'void Foo() {}'
                ]
            },
            compilation_database_files = ['src.cpp', 'fake_header_sources/src.cpp'],
            target_files = ['src.cpp', 'fake_header_sources/src.cpp'],
            additional_compile_flags = ['-I$TEMP_DIR'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR'],
            expected_errors = [])

    def test_function_declared_in_header_but_not_defined_in_source_module(self):
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                'src.h' : [
                    'void Foo();'
                ],
                'fake_header_sources/src.cpp': [
                    '#include "src.h"'
                ],
                'src.cpp': [
                    '#include "src.h"'
                ]
            },
            compilation_database_files = ['fake_header_sources/src.cpp', 'src.cpp'],
            target_files = ['fake_header_sources/src.cpp', 'src.cpp'],
            additional_compile_flags = ['-I$TEMP_DIR'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR'],
            expected_errors = [
                {
                    'msg': "Function 'Foo' declared but never defined",
                    'line': '1'
                }
            ])


    def test_function_declared_in_header_but_not_defined_in_source_module_reverse_processing_order(self):
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                'src.h' : [
                    'void Foo();'
                ],
                'fake_header_sources/src.cpp': [
                    '#include "src.h"'
                ],
                'src.cpp': [
                    '#include "src.h"'
                ]
            },
            compilation_database_files = ['src.cpp', 'fake_header_sources/src.cpp'],
            target_files = ['src.cpp', 'fake_header_sources/src.cpp'],
            additional_compile_flags = ['-I$TEMP_DIR'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR'],
            expected_errors = [
                {
                    'msg': "Function 'Foo' declared but never defined",
                    'line': '1'
                }
            ])

    def test_class_function(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '   void Bar();',
                '};'
            ],
            expected_errors = [
                {
                    'msg': "Function 'Foo::Bar' declared but never defined",
                    'line': '3'
                }
            ])

    def test_ignore_template_functions(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'template<typename T> void Foo();',
                'template<> void Foo<int>();'
            ],
            expected_errors = [])

    def test_ignore_deleted_functions(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '   Foo() = delete;',
                '};'
            ],
            expected_errors = [])

    def test_ignore_defaulted_functions(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '   Foo() = default;',
                '};'
            ],
            expected_errors = [])

    def test_ignore_pure_virtual_functions(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'struct Foo',
                '{',
                '   virtual void Bar() = 0;',
                '};'
            ],
            expected_errors = [])
