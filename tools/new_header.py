#!/usr/bin/env python3

import sys
from new_module import CModule

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f"""Usage: {sys.argv[0]} <module_path>""")
        exit(1)
    m = CModule(sys.argv[1])
    m.generate_header()
