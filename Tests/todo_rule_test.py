#!/usr/bin/env python3
import unittest
import test_support
import sys

class TestTodoRule(test_support.TestBase):
    def test_normal_comment(self):
        self.assert_colobot_lint_result(
            source_file_text = '// Comment',
            expected_errors = []
        )

    def test_single_todo_comment(self):
        self.assert_colobot_lint_result(
            source_file_text = '// TODO: comment',
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
            source_file_text = '// TODO: comment1\n// TODO: comment2',
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
            source_file_text = '/* comment */',
            expected_errors = [])

    def test_single_todo_star_comment(self):
        self.assert_colobot_lint_result(
            source_file_text = '/* TODO: comment */',
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
            source_file_text = 'int /* TODO: comment */ x;',
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
            source_file_text = '/* TODO: comment1\nSome text\nTODO: comment2 */',
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
    if len(sys.argv) >= 2:
        test_support.colobot_lint_exectuable = sys.argv.pop(1)

    unittest.main()