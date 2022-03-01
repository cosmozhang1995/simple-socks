#!/usr/bin/env python3

from cgi import test
from template import Template
import os

template = Template('Makefile.template')

basedir = os.path.join(os.path.dirname(__file__), '..', 'test')

testing_targets = []
for subdir in os.listdir(basedir):
    if os.path.isdir(os.path.join(basedir, subdir)):
        for filename in os.listdir(os.path.join(basedir, subdir)):
            if filename.endswith('.c') and os.path.isfile(os.path.join(basedir, subdir, filename)):
                testing_targets.append(os.path.join(subdir, filename.replace(r'\.c$', '')))

content = template.render(
    testing_targets = testing_targets
)

with open(os.path.join(os.path.dirname(__file__), '..', 'Makefile'), 'w') as f:
    f.write(content)
