#!/usr/bin/env python3
import unittest
import test_support
import sys

class TestTodoRule(test_support.TestBase):
    def setUp(self):
        self.set_rules_selection(['TodoRule'])

    def test_normal_comment(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '// Comment'
            ],
            expected_errors = []
        )

    def test_single_todo_comment(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '// TODO: comment'
            ],
            expected_errors = [
                {
                    'id': 'TODO comment',
                    'severity': 'information',
                    'msg': 'TODO: comment',
                    'line': '1'
                }
            ])

    def test_two_single_todo_comments(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '// TODO: comment1',
                '// TODO: comment2'
            ],
            expected_errors = [
                {
                    'id': 'TODO comment',
                    'severity': 'information',
                    'msg': 'TODO: comment1',
                    'line': '1'
                },
                {
                    'id': 'TODO comment',
                    'severity': 'information',
                    'msg': 'TODO: comment2',
                    'line': '2'
                }
            ])

    def test_normal_star_comment(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '/* comment */'
            ],
            expected_errors = [])

    def test_single_todo_star_comment(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '/* TODO: comment */'
            ],
            expected_errors = [
                {
                    'id': 'TODO comment',
                    'severity': 'information',
                    'msg': 'TODO: comment',
                    'line': '1'
                }
            ])

    def test_in_source_todo_star_comment(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'int /* TODO: comment */ x;'
            ],
            expected_errors = [
                {
                    'id': 'TODO comment',
                    'severity': 'information',
                    'msg': 'TODO: comment',
                    'line': '1'
                }
            ])

    def test_multiple_todo_star_comments(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                '/* TODO: comment1',
                'Some text',
                'TODO: comment2 */'
            ],
            expected_errors = [
                {
                    'id': 'TODO comment',
                    'severity': 'information',
                    'msg': 'TODO: comment1',
                    'line': '1'
                },
                {
                    'id': 'TODO comment',
                    'severity': 'information',
                    'msg': 'TODO: comment2',
                    'line': '3'
                }
            ])

if __name__ == '__main__':
    test_support.main()
