#!/usr/bin/env python3

import re
import os
from template import Template

template = Template('Makefile')

basedir = os.path.join(os.path.dirname(__file__), '..', 'test')

class TestingTarget:
    def __init__(self, name=None, bin_file=None, source_file=None):
        self.name = name
        self.bin_file = bin_file
        self.source_file = source_file

testing_targets = []
for subdir in os.listdir(basedir):
    if os.path.isdir(os.path.join(basedir, subdir)):
        for filename in os.listdir(os.path.join(basedir, subdir)):
            if filename.endswith('.c') and os.path.isfile(os.path.join(basedir, subdir, filename)):
                testing_targets.append(TestingTarget(
                    name = subdir + '_' + re.sub(r'\.c$', '', filename),
                    bin_file = subdir + '_' + re.sub(r'\.c$', '', filename),
                    source_file = os.path.join(subdir, filename)
                ))

content = template.render(
    testing_targets = testing_targets
)

with open(os.path.join(os.path.dirname(__file__), '..', 'Makefile'), 'w') as f:
    f.write(content)
