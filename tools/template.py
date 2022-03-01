import os
from jinja2 import Environment, FileSystemLoader, select_autoescape

class Template:
    def __init__(self, name):
        env = Environment(loader=FileSystemLoader(os.path.join(os.path.dirname(__file__), 'templates')),
                          autoescape=select_autoescape(),
                          line_statement_prefix='##')
        self.template = env.get_template(name)
    def render(self, **kwargs):
        return self.template.render(**kwargs)
