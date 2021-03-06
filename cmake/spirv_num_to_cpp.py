import argparse
import os
import re
import sys


SOURCE_FMT = """// AUTOGENERATED
// Generated by spirv_num_to_cpp.py, from '{file_name}'

#include <stddef.h>
#include <stdint.h>

namespace {namespace} {{

const volatile uint32_t {var_name}[] = {{
{raw_lines}
}};

const volatile size_t {var_name}_count = {total_data_count};

}} // {namespace}

// AUTOGENERATED
"""


HEADER_VARS_FMT = "extern const uint32_t {var_name}[];\nextern const size_t {var_name}_count;"


HEADER_FMT = """// AUTOGENERATED
// Generated by spirv_num_to_cpp.py

#pragma once

#include <stddef.h>
#include <stdint.h>

namespace {namespace} {{

{var_pairs}

}} // {namespace}

// AUTOGENERATED
"""


def readfile(path):
    with open(path) as f:
        return f.read()


def make_var_name(path):
    return os.path.basename(path).replace('.', '_')


def make_var_pairs(file_paths):
    pairs = (HEADER_VARS_FMT.format(var_name=make_var_name(path)) for path in file_paths)
    return '\n\n'.join(pairs)


def handle_source_command(args):
    file_data = readfile(args.src_path)

    file_name = os.path.basename(args.src_path)
    total_data_count = len(file_data.split(','))

    with open(os.path.join(args.dest_path, file_name + '.cpp'), 'w') as f:
        f.write(SOURCE_FMT.format(file_name=file_name, namespace=args.namespace, var_name=make_var_name(args.src_path),
                                  raw_lines=file_data, total_data_count=total_data_count))


def handle_header_command(args):
    with open(args.dest_path, 'w') as f:
        f.write(HEADER_FMT.format(namespace=args.namespace, var_pairs=make_var_pairs(args.spirv_files)))


def main():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='command')

    source_parser = subparsers.add_parser('source', help='generating spirv-in-cpp source files')
    source_parser.add_argument('-s', '--src-path', help='source file to stringify', required=True)
    source_parser.add_argument('-d', '--dest-path', help='generated files dir', required=True)
    source_parser.add_argument('-ns', '--namespace', help='namespace of the generated arguments', required=True)

    header_parser = subparsers.add_parser('header', help='generating spirv-in-cpp header file')
    header_parser.add_argument('-s', '--spirv-files', help='list of spirv files embedded in cpp', required=True, nargs='+')
    header_parser.add_argument('-d', '--dest-path', help='full file path for the generated header', required=True)
    header_parser.add_argument('-ns', '--namespace', help='namespace of the generated arguments', required=True)


    args = parser.parse_args()
    if args.command == 'source':
        handle_source_command(args)
    elif args.command == 'header':
        handle_header_command(args)
    else:
        print('Invalid command \'{command}\''.format(args.command), file=sys.stderr)
        return 1

    return 0

if __name__ == '__main__':
    sys.exit(main())
