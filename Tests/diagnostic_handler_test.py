import test_support

class DiagnosticHandlerTest(test_support.TestBase):
    def test_compile_warning_in_source_file(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'int Foo()',
                '{',
                '}',
                ''
            ],
            additional_compile_flags = ['-Wall'],
            expected_errors = [
                {
                    'id': 'compile warning',
                    'severity': 'warning',
                    'msg': "control reaches end of non-void function",
                    'line': '3'
                }
            ])

    def test_compile_error_in_source_file(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo()',
                '{',
                '   return Bar();',
                '}',
                ''
            ],
            additional_compile_flags = ['-Wall'],
            expected_errors = [
                {
                    'id': 'compile error',
                    'severity': 'error',
                    'msg': "use of undeclared identifier 'Bar'",
                    'line': '3'
                }
            ])

    def test_fatal_compile_error_in_source_file(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#include "nonexistent_include_file.h"',
                ''
            ],
            expected_errors = [
                {
                    'id': 'compile error',
                    'severity': 'error',
                    'msg': "'nonexistent_include_file.h' file not found",
                    'line': '1'
                }
            ])


    def test_compile_error_in_fake_header_source(self):
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                'foo/bar.h' : [
                    'Bar Foo() {}',
                    ''
                ],
                'fake_header_sources/foo/bar.cpp': [
                    '#include "foo/bar.h"',
                    ''
                ]
            },
            compilation_database_files = ['fake_header_sources/foo/bar.cpp'],
            target_files = ['fake_header_sources/foo/bar.cpp'],
            additional_compile_flags = ['-I$TEMP_DIR'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR'],
            expected_errors = [
                {
                    'id': 'header file not self-contained',
                    'severity': 'error',
                    'msg': "Including single header file should not result in compile error: unknown type name 'Bar'",
                    'line': '1'
                }
            ])

    def test_print_only_unique_compile_warnings(self):
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                'header.h': [
                    'int Foo()',
                    '{',
                    '}',
                    ''
                ],
                'src1.cpp': [
                    '#include "header.h"',
                    ''
                ],
                'src2.cpp': [
                    '#include "header.h"',
                    ''
                ]
            },
            compilation_database_files = ['src1.cpp', 'src2.cpp'],
            target_files = ['src1.cpp', 'src2.cpp'],
            additional_compile_flags = ['-Wall'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR'],
            expected_errors = [
                {
                    'id': 'compile warning',
                    'severity': 'warning',
                    'msg': "control reaches end of non-void function",
                    'line': '3'
                }
            ])
