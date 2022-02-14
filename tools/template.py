import imp
import os
import re

class Template:
    def __init__(self, name):
        with open(os.path.join(os.path.dirname(__file__), 'templates', name), 'r') as f:
            self.template = f.read()
    def render(self, **kwargs):
        if len(kwargs) == 0:
            return self.template
        return re.compile('\\b(' + '|'.join([k for k in kwargs.keys()]) + ')\\b')\
            .sub(lambda m: kwargs[m.group(1)], self.template)
