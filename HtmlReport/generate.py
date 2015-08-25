#!/usr/bin/env python3
import argparse
import os
import shutil
import cgi
import xml.etree.ElementTree as ET

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--xml-report-file', required=True, help='colobot-lint report in XML format')
    parser.add_argument('--output-dir', required=True, help='output directory')
    return parser.parse_args()

def copy_dir(src_dir, dest_dir, dir_name):
    shutil.rmtree(dest_dir + '/' + dir_name, ignore_errors=True)
    shutil.copytree(src_dir + '/' + dir_name, dest_dir + '/' + dir_name)

def prepare_dirs(output_dir):
    os.makedirs(output_dir, exist_ok=True)

    this_dir = os.path.dirname(os.path.realpath(__file__))
    copy_dir(this_dir, output_dir, 'js')
    copy_dir(this_dir, output_dir, 'css')
    copy_dir(this_dir, output_dir, 'img')

def write_html(errors, output_file_name):
    with open(output_file_name, 'w') as output_file:
        def out(text):
            output_file.write(text + '\n')

        out('<!DOCTYPE html>')
        out('<html>')
        write_head(out)
        out('<body>')
        write_table(errors, out)
        out('</body>')
        out('</html>')

def write_head(out):
    out('<head>')
    out('  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">')
    out('  <title>Colobot-lint HTML report</title>')
    out('  <link rel="stylesheet" type="text/css" href="css/normalize.css">')
    out('  <link rel="stylesheet" type="text/css" href="css/style.css">')
    out('  <script type="text/javascript" src="js/jquery.min.js"></script>')
    out('  <script type="text/javascript" src="js/jquery-sort-elements.js"></script>')
    out('  <script type="text/javascript" src="js/interactions.js"></script>')
    out('</head>')

def write_table(errors, out):
    out('<table id="errors">')

    write_colgroup(out)

    out('<thead>')
    write_thead_headings(out)
    write_thead_filters(errors, out)
    out('</thead>')

    out('<tbody>')
    project_prefix = get_project_prefix(errors)
    for error in errors:
        write_tbody_row(error, project_prefix, out)
    out('</tbody>')

    out('</table>')

def write_colgroup(out):
    out('<colgroup>')
    out('  <col span="1" style="width: 21%;">')
    out('  <col span="1" style="width: 18%;">')
    out('  <col span="1" style="width: 11%;">')
    out('  <col span="1" style="width: 50%;">')
    out('</colgroup>')

def write_thead_headings(out):
    out('<tr class="headings">')
    out('  <th class="location">Location <span class="sort-indicator"></span></th>')
    out('  <th class="category">Category <span class="sort-indicator"></span></th>')
    out('  <th class="severity">Severity <span class="sort-indicator"></span></th>')
    out('  <th class="message">Message <span class="sort-indicator"></span></th>')
    out('</tr>')

def write_thead_filters(errors, out):
    unique_categories = set()
    unique_severities = set()
    for error in errors:
        unique_categories.add(error.get('id'))
        unique_severities.add(error.get('severity'))

    out('<tr class="filters">')
    out('  <th class="location"><input class="filter" placeholder="Regex search..."></th>')

    out('  <th class="category">')
    out('    <select class="filter">')
    out('      <option>(all)</option>')
    for unique_category in unique_categories:
        out('      <option>%s</option>' % cgi.escape(unique_category))
    out('    </select>')
    out('  </th>')

    out('  <th class="severity">')
    out('    <select class="filter">')
    out('      <option>(all)</option>')
    for unique_severity in unique_severities:
        out('    <option>%s</option>' % cgi.escape(unique_severity))
    out('    </select>')
    out('  </th>')

    out('  <th class="message"><input class="filter" placeholder="Regex search..."></th>')
    out('</tr>')

def get_project_prefix(errors):
    project_prefix = None

    for error in errors:
        location = error.find('location')
        file_name = location.get('file')
        if project_prefix is None:
            project_prefix = file_name
        else:
            project_prefix = os.path.commonprefix([project_prefix, file_name])

    return project_prefix

def write_tbody_row(error, project_prefix, out):
    out('<tr>')

    location = error.find('location')
    file_name = os.path.relpath(location.get('file'), start=project_prefix)
    location_str = file_name + ":" + location.get('line')

    out('  <td class="location">%s</td>' % cgi.escape(location_str))
    out('  <td class="category">%s</td>' % cgi.escape(error.get('id')))
    out('  <td class="severity">%s</td>' % cgi.escape(error.get('severity')))
    out('  <td class="message">%s</td>' % cgi.escape(error.get('msg')))

    out('</tr>')


def main():
    options = parse_args()

    prepare_dirs(options.output_dir)

    results = ET.parse(options.xml_report_file)
    errors = results.find('errors').findall('error')

    html_file = options.output_dir + '/index.html'

    write_html(errors, html_file)

if __name__ == '__main__':
    main()
