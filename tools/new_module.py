#!/usr/bin/env python3

import sys
import os
import re
from template import Template

class CModule:
    def __init__(self, module_path):
        self.module_path = sys.argv[1]

        self.header_file = module_path + '.h'
        self.source_file = module_path + '.c'

        header_file_relpath = os.path.relpath(
            os.path.realpath(module_path),
            os.path.realpath(os.path.join(os.path.dirname(__file__), '..', 'src'))
        )
        if header_file_relpath.startswith('..'):
            header_file_relpath = os.path.relpath(
                os.path.realpath(module_path),
                os.path.realpath(os.path.join(os.path.dirname(__file__), '..', 'test'))
            )
        if header_file_relpath.startswith('..'):
            raise Exception(f"module [{module_path}] should be either in the 'src' or the 'test' directory")

        self.module_def = '_' + '_'.join(re.split(r'[\/\\]+', header_file_relpath)).upper() + '_H_'
        if not self.module_def.startswith('_SS_'):
            self.module_def = '_SS' + self.module_def

        if not re.match(r'^[A-Z0-9_]+$', self.module_def):
            raise Exception(f"invalid module path [{module_path}]")
        
        self.header_file_content = None
        self.source_file_content = None

    def generate_folder(self):
        file_dir = os.path.dirname(self.module_path)
        if not os.path.isdir(file_dir):
            os.makedirs(file_dir)

    def compile_header(self):
        if self.header_file_content is None:
            self.header_file_content = Template('module.h').render(
                module_def = self.module_def
            )
    
    def compile_source(self):
        if self.source_file_content is None:
            self.source_file_content = None

    def generate_header(self):
        self.compile_header()
        self.generate_folder()
        if os.path.exists(self.header_file):
            raise Exception(f"file [{self.header_file}] already exists.")
        with open(self.header_file, 'w') as f:
            f.write(self.header_file_content)
    
    def generate_source(self):
        self.compile_source()
        self.generate_folder()
        if os.path.exists(self.source_file):
            raise Exception(f"file [{self.source_file}] already exists.")
        with open(self.source_file, 'w') as f:
            f.write(self.source_file_content)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("""Usage: {sys.argv[0]} <module_path>""")
        exit(1)
    m = CModule(sys.argv[1])
    m.compile_header()
    m.compile_source()
    m.generate_header()
    m.generate_source()
