#!/usr/bin/env python3
import argparse
import re
import sys
import xml.etree.ElementTree as ET

def parse_args():
    parser = argparse.ArgumentParser(description='Count the number of errors of given type and in given files in colobot-lint XML report file')
    parser.add_argument('--file-filter', required=False, default='.*', help='regex filter for file paths (precede with "-" to inverse)')
    parser.add_argument('--rule-filter', required=False, default='.*', help='regex filter for rule names (precede with "-" to inverse)')
    parser.add_argument('--xml-report-file', required=True, help='colobot-lint report in XML format')
    return parser.parse_args()

def matches_regex(pattern, text):
    # Temporary fix, see issue #14
    if not text:
        return False

    if pattern.startswith('-'):
        return not re.match(pattern[1:], text)
    else:
        return re.match(pattern, text)

def main():
    options = parse_args()

    results = ET.parse(options.xml_report_file)
    errors = results.find('errors').findall('error')

    count = 0
    for error in errors:
        location = error.find('location')
        if (matches_regex(options.file_filter, location.get('file')) and
            matches_regex(options.rule_filter, error.get('id'))):
            count = count + 1

    sys.stdout.write('{0}\n'.format(count))

if __name__ == '__main__':
    main()
