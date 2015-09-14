import test_support

class LicenseInHeaderRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['LicenseInHeaderRule'])
        self.set_default_error_id('license header')
        self.set_default_error_severity('style')

    def assert_colobot_lint_result_with_license_file(self,
                                                     source_file_lines,
                                                     license_file_lines,
                                                     expected_errors):
        main_file = 'src.cpp'
        license_file = 'license.txt'
        source_files_data = {
            main_file: source_file_lines,
            license_file: license_file_lines }

        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = source_files_data,
            compilation_database_files = [main_file],
            target_files = [main_file],
            additional_options = ['-license-template-file', '$TEMP_DIR/license.txt'],
            expected_errors = expected_errors)

    def test_no_license_file_supplied(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void Foo()',
                '{',
                '    int x = 0;',
                '}',
                ''
            ],
            expected_errors = [])

    def test_correct_license_header(self):
        self.assert_colobot_lint_result_with_license_file(
            source_file_lines = [
                '/* Copyright',
                '   Bla bla bla',
                '*/',
                'void Foo()',
                '{',
                '    int x = 0;',
                '}',
                ''
            ],
            license_file_lines = [
                '/* Copyright',
                '   Bla bla bla',
                '*/'
            ],
            expected_errors = [])

    def test_incorrect_license_header(self):
        self.assert_colobot_lint_result_with_license_file(
            source_file_lines = [
                '/* Copyright',
                '   Bla',
                '*/',
                'void Foo()',
                '{',
                '    int x = 0;',
                '}',
                ''
            ],
            license_file_lines = [
                '/* Copyright',
                '   Bla bla bla',
                '*/'
            ],
            expected_errors = [
                {
                    'msg': "File doesn't have proper license header; expected line was '   Bla bla bla'",
                    'line': '2'
                }
            ])

    def test_incorrect_license_header_in_fake_header(self):
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                'src.h' : [
                    '/* Copyright',
                    '   Bla',
                    '*/',
                    'void Foo()',
                    '{',
                    '    int x = 0;',
                    '}',
                    ''
                ],
                'fake_header_sources/src.cpp': [
                    '#include "src.h"'
                ],
                'license.txt': [
                    '/* Copyright',
                    '   Bla bla bla',
                    '*/'
                ]
            },
            compilation_database_files = ['fake_header_sources/src.cpp'],
            target_files = ['fake_header_sources/src.cpp'],
            additional_compile_flags = ['-I$TEMP_DIR'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR', '-license-template-file', '$TEMP_DIR/license.txt'],
            expected_errors = [
                {
                    'msg': "File doesn't have proper license header; expected line was '   Bla bla bla'",
                    'line': '2'
                }
            ])
