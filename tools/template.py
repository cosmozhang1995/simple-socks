import os
from jinja2 import Environment, FileSystemLoader, select_autoescape

class Template:
    def __init__(self, name):
        with open(os.path.join(os.path.dirname(__file__), 'templates', name), 'r') as f:
            self.template = f.read()
    def render(self, **kwargs):
        if len(kwargs) == 0:
            return self.template
        return re.compile('\\b(' + '|'.join([k for k in kwargs.keys()]) + ')\\b')\
            .sub(lambda m: kwargs[m.group(1)], self.template)

class Template:
    def __init__(self, name):
        env = Environment(loader=FileSystemLoader(os.path.join(os.path.dirname(__file__), 'templates')),
                          autoescape=select_autoescape(),
                          line_statement_prefix='##')
        self.template = env.get_template(name)
    def render(self, **kwargs):
        return self.template.render(**kwargs)
