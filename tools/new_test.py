#!/usr/bin/env python3

import os
import sys
from template import Template


class CTestModule:
    def __init__(self, module_path):
        self.module_path = module_path

        module_rel_path = os.path.relpath(
            os.path.realpath(module_path),
            os.path.realpath(os.path.join(os.path.dirname(__file__), '..', 'test'))
        )
        if module_rel_path.startswith('..'):
            raise Exception(f"module [{module_path}] should be either in the 'test' directory")
        module_name = os.path.basename(module_path)
        if '.' in module_name:
            raise Exception(f"invalid module name [{module_name}]")

        self.source_file_path = module_path + '.c'

        self.source_content = None

    def generate_folder(self):
        file_dir = os.path.dirname(self.module_path)
        if not os.path.isdir(file_dir):
            os.makedirs(file_dir)

    def compile_source(self):
        if not self.source_content:
            self.source_content = Template('unittest.c').render()

    def generate_source(self):
        self.compile_source()
        self.generate_folder()
        if os.path.exists(self.source_file_path):
            raise Exception(f"file [{self.source_file_path}] already exists.")
        with open(self.source_file_path, 'w') as f:
            f.write(self.source_content)


if __name__ == '__main__':
    module = CTestModule(sys.argv[1])
    module.generate_source()

