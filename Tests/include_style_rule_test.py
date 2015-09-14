import test_support
import os

class IncludeStyleRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['IncludeStyleRule'])
        self.set_default_error_id('include style')
        self.set_default_error_severity('style')

    def assert_colobot_lint_result_with_project_headers(self,
                                                        main_file_lines,
                                                        main_file,
                                                        project_header_files,
                                                        system_header_files,
                                                        expected_errors):
        main_file = os.path.join('project', main_file)
        source_files_data = { main_file: main_file_lines }

        for project_header_file in project_header_files:
            source_files_data[os.path.join('project', project_header_file)] = []

        for system_header_file in system_header_files:
            source_files_data[os.path.join('system', system_header_file)] = []

        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = source_files_data,
            compilation_database_files = [main_file],
            target_files = [main_file],
            additional_compile_flags = ['-I$TEMP_DIR/project', '-I$TEMP_DIR/system'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR/project'],
            expected_errors = expected_errors)

    def assert_colobot_lint_result_with_project_headers_and_fake_header_source(self,
                                                                               main_file_lines,
                                                                               main_file,
                                                                               project_headers,
                                                                               system_header_files,
                                                                               expected_errors):
        fake_header_sources_dir = os.path.join('project', 'fake_header_sources')

        source_files_data = dict()
        main_file_without_ext, _ = os.path.splitext(main_file)
        fake_header_source_file = os.path.join(fake_header_sources_dir, main_file_without_ext + '.cpp')
        source_files_data[fake_header_source_file] = ['#include "{0}"'.format(main_file)]

        main_file = os.path.join('project', main_file)
        source_files_data[main_file] = main_file_lines

        for project_header_file in project_headers.keys():
            source_files_data[os.path.join('project', project_header_file)] = project_headers[project_header_file]

        for system_header_file in system_header_files:
            source_files_data[os.path.join('system', system_header_file)] = []

        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = source_files_data,
            compilation_database_files = [fake_header_source_file],
            target_files = [fake_header_source_file],
            additional_compile_flags = ['-I$TEMP_DIR/project', '-I$TEMP_DIR/system', '-I$TEMP_DIR/' + fake_header_sources_dir],
            additional_options = ['-project-local-include-path', '$TEMP_DIR/project'],
            expected_errors = expected_errors)

    def test_no_includes(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                ''
            ],
            expected_errors = [])

    def test_local_includes_sorted_alphabetically(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "abc.h"',
                '#include "def.h"',
                '',
                '#include "def/abc.h"',
                '#include "def/def.h"',
            ],
            main_file = 'src.cpp',
            project_header_files = [
                'abc.h',
                'def.h',
                'def/abc.h',
                'def/def.h'
            ],
            system_header_files = [],
            expected_errors = [])

    def test_local_includes_not_sorted_alphabetically(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "abc.h"',
                '#include "def.h"',
                '',
                '#include "def/def.h"',
                '#include "def/abc.h"',
            ],
            main_file = 'src.cpp',
            project_header_files = [
                'abc.h',
                'def.h',
                'def/abc.h',
                'def/def.h'
            ],
            system_header_files = [],
            expected_errors = [
                {
                    'msg': "Broken alphabetical ordering, expected 'def/abc.h', not 'def/def.h'",
                    'line': '4'
                }
            ])

    def test_local_includes_from_different_subpaths_in_one_block(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "abc.h"',
                '#include "def.h"',
                '#include "def/abc.h"',
                '#include "def/def.h"',
            ],
            main_file = 'src.cpp',
            project_header_files = [
                'abc.h',
                'def.h',
                'def/abc.h',
                'def/def.h'
            ],
            system_header_files = [],
            expected_errors = [
                {
                    'msg': "Expected empty line between include directives",
                    'line': '3'
                }
            ])

    def test_system_includes_dont_need_to_be_sorted_alphabetically(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include <system_header2.h>',
                '#include <system_header1.h>'
            ],
            main_file = 'src.cpp',
            project_header_files = [],
            system_header_files = [
                'system_header1.h',
                'system_header2.h'
            ],
            expected_errors = [])

    def test_local_include_in_angle_brackets(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "abc.h"',
                '#include "def.h"',
                '',
                '#include <def/abc.h>',
                '#include "def/def.h"',
            ],
            main_file = 'src.cpp',
            project_header_files = [
                'abc.h',
                'def.h',
                'def/abc.h',
                'def/def.h'
            ],
            system_header_files = [],
            expected_errors = [
                {
                    'msg': "Local include 'def/abc.h' should be included with quotes, not angled brackets",
                    'line': '4'
                }
            ])

    def test_global_include_in_quotes(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include <system_header1.h>',
                '#include "system_header2.h"',
                '#include <system_header3.h>',
            ],
            main_file = 'src.cpp',
            project_header_files = [],
            system_header_files = [
                'system_header1.h',
                'system_header2.h',
                'system_header3.h',
            ],
            expected_errors = [
                {
                    'msg': "Global include 'system_header2.h' should be included with angled brackets, not quotes",
                    'line': '2'
                }
            ])

    def test_local_include_not_full_path_from_project_root(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "abc.h"',
                '#include "def.h"',
                '#include "jkl.h"',
                '',
                '#include "def/ghi.h"',
            ],
            main_file = 'def/mno.cpp',
            project_header_files = [
                'abc.h',
                'def.h',
                'def/ghi.h',
                'def/jkl.h'
            ],
            system_header_files = [],
            expected_errors = [
                {
                    'msg': "Expected local include to be full relative path from project local include search path: 'def/jkl.h', not 'jkl.h'",
                    'line': '3'
                }
            ])

    def test_local_and_global_includes_in_one_block(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "abc.h"',
                '#include "def.h"',
                '#include <system_header1.h>',
                '#include <system_header2.h>'
            ],
            main_file = 'def/ghi.cpp',
            project_header_files = [
                'abc.h',
                'def.h'
            ],
            system_header_files = [
                'system_header1.h',
                'system_header2.h',
            ],
            expected_errors = [
                {
                    'msg': "Expected empty line between include directives",
                    'line': '3'
                }
            ])

    def test_local_and_global_includes_in_separate_blocks(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "abc.h"',
                '#include "def.h"',
                '',
                '#include <system_header1.h>',
                '#include <system_header2.h>',
            ],
            main_file = 'def/ghi.cpp',
            project_header_files = [
                'abc.h',
                'def.h'
            ],
            system_header_files = [
                'system_header1.h',
                'system_header2.h'
            ],
            expected_errors = [])

    def test_local_include_after_global_include(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "abc.h"',
                '',
                '#include <system_header1.h>',
                '#include <system_header2.h>',
                '#include "def.h"'
            ],
            main_file = 'def/ghi.cpp',
            project_header_files = [
                'abc.h',
                'def.h'
            ],
            system_header_files = [
                'system_header1.h',
                'system_header2.h'
            ],
            expected_errors = [
                {
                    'msg': "Local include 'def.h' should not be placed after global includes",
                    'line': '5'
                }
            ])

    def test_config_header_at_the_top(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "config/config.h"',
                '',
                '#include "abc.h"',
                '#include "def.h"',
                '',
                '#include "def/abc.h"',
                '#include "def/def.h"',
            ],
            main_file = 'src.cpp',
            project_header_files = [
                'config/config.h',
                'abc.h',
                'def.h',
                'def/abc.h',
                'def/def.h'
            ],
            system_header_files = [],
            expected_errors = [])

    def test_config_header_in_one_block_at_the_top(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "config/config.h"',
                '#include "abc.h"',
                '#include "def.h"',
                '',
                '#include "def/abc.h"',
                '#include "def/def.h"',
            ],
            main_file = 'src.cpp',
            project_header_files = [
                'config/config.h',
                'abc.h',
                'def.h',
                'def/abc.h',
                'def/def.h'
            ],
            system_header_files = [],
            expected_errors = [
                {
                    'msg': "Expected empty line between include directives",
                    'line': '2'
                }
            ])

    def test_config_header_not_at_the_top(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "abc.h"',
                '#include "def.h"',
                '',
                '#include "config/config.h"',
                '',
                '#include "def/abc.h"',
                '#include "def/def.h"',
            ],
            main_file = 'src.cpp',
            project_header_files = [
                'config/config.h',
                'abc.h',
                'def.h',
                'def/abc.h',
                'def/def.h'
            ],
            system_header_files = [],
            expected_errors = [
                {
                    'msg': "Expected config include directive: 'config/config.h', not 'abc.h'",
                    'line': '1'
                },
                {
                    'msg': "Expected empty line between include directives",
                    'line': '2'
                }
            ])

    def test_matching_header_at_the_top(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "src.h"',
                '',
                '#include "abc.h"',
                '#include "def.h"',
                '',
                '#include "def/abc.h"',
                '#include "def/def.h"',
            ],
            main_file = 'src.cpp',
            project_header_files = [
                'src.h',
                'abc.h',
                'def.h',
                'def/abc.h',
                'def/def.h'
            ],
            system_header_files = [],
            expected_errors = [])

    def test_matching_header_in_one_block(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "src.h"',
                '#include "abc.h"',
                '#include "def.h"',
                '',
                '#include "def/abc.h"',
                '#include "def/def.h"',
            ],
            main_file = 'src.cpp',
            project_header_files = [
                'src.h',
                'abc.h',
                'def.h',
                'def/abc.h',
                'def/def.h'
            ],
            system_header_files = [],
            expected_errors = [
                {
                    'msg': "Expected empty line between include directives",
                    'line': '2'
                }
            ])

    def test_matching_header_and_config_file(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "src.h"',
                '',
                '#include "config.h"',
                '',
                '#include "abc.h"',
                '#include "def.h"',
                '',
                '#include "def/abc.h"',
                '#include "def/def.h"',
            ],
            main_file = 'src.cpp',
            project_header_files = [
                'config.h',
                'src.h',
                'abc.h',
                'def.h',
                'def/abc.h',
                'def/def.h'
            ],
            system_header_files = [],
            expected_errors = [])

    def test_matching_header_not_at_the_top(self):
        self.assert_colobot_lint_result_with_project_headers(
            main_file_lines = [
                '#include "abc.h"',
                '#include "src.h"',
                '#include "def.h"',
                '',
                '#include "def/abc.h"',
                '#include "def/def.h"',
            ],
            main_file = 'src.cpp',
            project_header_files = [
                'src.h',
                'abc.h',
                'def.h',
                'def/abc.h',
                'def/def.h'
            ],
            system_header_files = [],
            expected_errors = [
                {
                    'msg': "Expected first include directive to be matching header file: 'src.h', not 'abc.h'",
                    'line': '1'
                },
                {
                    'msg': "Expected empty line between include directives",
                    'line': '2'
                },
                {
                    'msg': "Broken alphabetical ordering, expected 'def.h', not 'src.h'",
                    'line': '2'
                }
            ])

    def test_base_class_header_at_the_top(self):
        self.assert_colobot_lint_result_with_project_headers_and_fake_header_source(
            main_file_lines = [
                '#include "def/base.h"',
                '',
                '#include "abc/abc.h"',
                '#include "abc/def.h"',
                '',
                '#include "def/abc.h"',
                '#include "def/def.h"',
                '',
                'class Derived : public Base {};'
            ],
            main_file = 'def/src.h',
            project_headers = {
                'abc/abc.h': [],
                'abc/def.h': [],
                'def/abc.h': [],
                'def/def.h': [],
                'def/base.h': [
                    'class Base {};'
                ]
            },
            system_header_files = [],
            expected_errors = [])

    def test_base_class_header_in_one_block(self):
        self.assert_colobot_lint_result_with_project_headers_and_fake_header_source(
            main_file_lines = [
                '#include "def/base.h"',
                '#include "abc/abc.h"',
                '#include "abc/def.h"',
                '',
                '#include "def/abc.h"',
                '#include "def/def.h"',
                '',
                'class Derived : public Base {};'
            ],
            main_file = 'def/src.h',
            project_headers = {
                'abc/abc.h': [],
                'abc/def.h': [],
                'def/abc.h': [],
                'def/def.h': [],
                'def/base.h': [
                    'class Base {};'
                ]
            },
            system_header_files = [],
            expected_errors = [
                {
                    'id': 'include style',
                    'severity': 'style',
                    'msg': "Expected empty line between include directives",
                    'line': '2'
                }
            ])

    def test_base_class_header_and_config_file(self):
        self.assert_colobot_lint_result_with_project_headers_and_fake_header_source(
            main_file_lines = [
                '#include "def/base.h"',
                '',
                '#include "config/config.h"',
                '',
                '#include "abc/abc.h"',
                '#include "abc/def.h"',
                '',
                '#include "def/abc.h"',
                '#include "def/def.h"',
                '',
                '#include <system_header.h>',
                '',
                'class Derived : public Base {};'
            ],
            main_file = 'def/src.h',
            project_headers = {
                'config/config.h': [],
                'abc/abc.h': [],
                'abc/def.h': [],
                'def/abc.h': [],
                'def/def.h': [],
                'def/base.h': [
                    'class Base {};'
                ]
            },
            system_header_files = [
                'system_header.h'
            ],
            expected_errors = [])
