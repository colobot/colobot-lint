import test_support

class SourceLocationHelperTest(test_support.TestBase):
    def test_ignore_macro_body_expansion(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#define DEFINE_FUNC() \\',
                '  void deleteMe(int* x) \\',
                '  { \\',
                '     delete x; \\',
                '  }',
                'DEFINE_FUNC();'
            ],
            expected_errors = [],
            rules_selection = ['NakedDeleteRule'])

    def test_ignore_macro_argument_expansion(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '#define DEFINE_FUNC_WITH_INSTR(instr) \\',
                '  void deleteMe(int* x) \\',
                '  { \\',
                '     instr; \\',
                '  }',
                'DEFINE_FUNC_WITH_INSTR(delete x);'
            ],
            expected_errors = [],
            rules_selection = ['NakedDeleteRule'])

    def test_fake_header_source(self):
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                'fake_header_sources/foo/bar.cpp': [
                    '#include "foo/bar.h"',
                    '#include "foo/baz.h"'
                ],
                # report violations in this header
                'foo/bar.h': [
                    'void deleteMe(int* x)',
                    '{',
                    '  delete x;',
                    '}'
                ],
                # but not this one:
                'foo/baz.h': [
                    'int* createMe()',
                    '{',
                    '  return new int;',
                    '}'
                ]
            },
            compilation_database_files = ['fake_header_sources/foo/bar.cpp'],
            target_files = ['fake_header_sources/foo/bar.cpp'],
            additional_compile_flags = ['-I$TEMP_DIR'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR'],
            rules_selection = ['NakedNewRule', 'NakedDeleteRule'],
            expected_errors = [
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'int'",
                    'line': '3'
                }
            ])

    def test_exclusion_zone_exclude_one_rule(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void deleteMe1(int* x) { delete x; }',
                '// @colobot-lint-exclude NakedDeleteRule',
                'void deleteMe2(int* x) { delete x; }',
                '// @end-colobot-lint-exclude',
                'void deleteMe3(int* x) { delete x; }'
            ],
            expected_errors = [
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'int'",
                    'line': '1'
                },
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'int'",
                    'line': '5'
                }
            ],
            rules_selection = ['NakedDeleteRule'])

    def test_exclusion_zone_exclude_two_rules(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void deleteMe1(int* x) { delete x; }',
                'int* createMe1() { return new int(); }',
                '// @colobot-lint-exclude NakedDeleteRule NakedNewRule',
                'void deleteMe2(int* x) { delete x; }',
                'int* createMe2() { return new int(); }',
                '// @end-colobot-lint-exclude',
                'void deleteMe3(int* x) { delete x; }',
                'int* createMe3() { return new int(); }'
            ],
            expected_errors = [
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'int'",
                    'line': '1'
                },
                {
                    'id': 'naked new',
                    'severity': 'warning',
                    'msg': "Naked new called with type 'int'",
                    'line': '2'
                },
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'int'",
                    'line': '7'
                },
                {
                    'id': 'naked new',
                    'severity': 'warning',
                    'msg': "Naked new called with type 'int'",
                    'line': '8'
                }
            ],
            rules_selection = ['NakedDeleteRule', 'NakedNewRule'])

    def test_exclusion_zone_exclude_all_rules_with_star(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'void deleteMe1(int* x) { delete x; }',
                'int* createMe1() { return new int(); }',
                '// @colobot-lint-exclude *',
                'void deleteMe2(int* x) { delete x; }',
                'int* createMe2() { return new int(); }',
                '// @end-colobot-lint-exclude',
                'void deleteMe3(int* x) { delete x; }',
                'int* createMe3() { return new int(); }'
            ],
            expected_errors = [
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'int'",
                    'line': '1'
                },
                {
                    'id': 'naked new',
                    'severity': 'warning',
                    'msg': "Naked new called with type 'int'",
                    'line': '2'
                },
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'int'",
                    'line': '7'
                },
                {
                    'id': 'naked new',
                    'severity': 'warning',
                    'msg': "Naked new called with type 'int'",
                    'line': '8'
                }
            ],
            rules_selection = ['NakedDeleteRule', 'NakedNewRule'])
