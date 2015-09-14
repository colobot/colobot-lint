import test_support

class BeginSourceFileHandlerTest(test_support.TestBase):
    def test_process_only_unique_files_from_compilation_database(self):
        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = {
                'src.cpp' : [
                    'void deleteMe(int* x)',
                    '{',
                    '   delete x;',
                    '}'
                ]
            },
            compilation_database_files = ['src.cpp', 'src.cpp'],
            target_files = ['src.cpp'],
            rules_selection = ['NakedDeleteRule'],
            expected_errors = [
                {
                    'id': 'naked delete',
                    'severity': 'warning',
                    'msg': "Naked delete called on type 'int'",
                    'line': '3'
                }
            ])
