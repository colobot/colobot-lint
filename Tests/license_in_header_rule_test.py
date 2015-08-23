import test_support
import os

class LicenseInHeaderRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['LicenseInHeaderRule'])
        self.set_default_error_id('license header')
        self.set_default_error_severity('style')

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

    def assert_colobot_lint_result_with_license_file(self,
                                                    source_file_lines,
                                                    license_file_lines,
                                                    expected_errors):
        with test_support.TempBuildDir() as temp_dir:
            source_file_name = temp_dir + '/src.cpp'
            test_support.write_file_lines(source_file_name, source_file_lines)

            license_template_file_name = temp_dir + '/license.txt'
            test_support.write_file_lines(license_template_file_name, license_file_lines)

            test_support.write_compilation_database(
                build_directory = temp_dir,
                source_file_names = [source_file_name])

            xml_output = test_support.run_colobot_lint(build_directory = temp_dir,
                                                       source_dir = temp_dir,
                                                       source_paths = [source_file_name],
                                                       rules_selection = ['LicenseInHeaderRule'],
                                                       additional_options = ['-license-template-file', license_template_file_name])
            self.assert_xml_output_match(
                xml_output = xml_output,
                expected_errors = expected_errors)

    def test_incorrect_license_header_in_fake_header(self):
        with test_support.TempBuildDir() as temp_dir:
            os.mkdir(temp_dir + '/fake_header_sources')

            hpp_file_name = temp_dir + '/src.h'
            test_support.write_file_lines(hpp_file_name, [
                    '/* Copyright',
                    '   Bla',
                    '*/',
                    'void Foo()',
                    '{',
                    '    int x = 0;',
                    '}',
                    ''
                ]
            )

            cpp_file_name = temp_dir + '/fake_header_sources/src.cpp'
            test_support.write_file_lines(cpp_file_name, [
                    '#include "src.h"'
                ]
            )

            license_template_file_name = temp_dir + '/license.txt'
            test_support.write_file_lines(license_template_file_name, [
                    '/* Copyright',
                    '   Bla bla bla',
                    '*/'
                ]
            )

            test_support.write_compilation_database(
                build_directory = temp_dir,
                source_file_names = [cpp_file_name],
                additional_compile_flags = '-I' + temp_dir)

            xml_output = test_support.run_colobot_lint(build_directory = temp_dir,
                                                       source_dir = temp_dir,
                                                       source_paths = [cpp_file_name],
                                                       rules_selection = ['LicenseInHeaderRule'],
                                                       additional_options = ['-license-template-file', license_template_file_name])
            self.assert_xml_output_match(
                xml_output = xml_output,
                expected_errors = [
                    {
                        'msg': "File doesn't have proper license header; expected line was '   Bla bla bla'",
                        'line': '2'
                    }
                ]
            )
