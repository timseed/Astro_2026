"""Catalog Table integration for Sphinx.

Author: Valentin Boettcher <hiro@protagon.space>
"""

import builder
from docutils import nodes
from sphinx.util.docutils import SphinxDirective
from html.parser import HTMLParser


class HTMLFilter(HTMLParser):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.current_node = nodes.paragraph()

    def handle_starttag(self, tag, attrs):
        if tag == "a":
            attrs = dict(attrs)
            href = attrs["href"] if "href" in attrs.keys() else "#"
            node = nodes.reference(internal=False, refuri=href)
        else:
            node = nodes.paragraph()
        self.current_node += node
        self.current_node = node

    def handle_endtag(self, tag):
        self.current_node = self.current_node.parent

    def handle_data(self, data):
        self.current_node += nodes.paragraph(text=data)

    def get_tag(self):
        return self.current_node


class CatalogtableDirective(SphinxDirective):
    """
    A simple directive to download an parse markdown tag release notes
    embdedding them into the document.
    """

    fields = {
        "id": ("ID", 1),
        "name": ("Name", 2),
        "version": ("Version", 1),
        "precedence": ("Precedence", 1),
        "description": ("Description", 4),
        "author": ("Author", 2),
        "source": ("Source", 1),
    }

    has_content = True

    def __init__(self, *args, **kwargs):
        self._catalogs = builder.get_catalogs_df().sort_values("id")

        super().__init__(*args, **kwargs)

    def run(self):
        """Main entry for the directive."""

        table = nodes.table()
        tgroup = nodes.tgroup(cols=1)
        table += tgroup

        tbody = nodes.tbody()
        thead = nodes.tbody()

        for field, (_, width) in self.fields.items():
            colspec = nodes.colspec(colwidth=width)
            tgroup += colspec

        head_row = nodes.row()
        for _, (name, _) in self.fields.items():
            entry = nodes.entry()
            head_row += entry
            entry += nodes.paragraph(text=name)

        thead += head_row
        tgroup += thead
        tgroup += tbody

        for _, cat in self._catalogs.iterrows():
            row = nodes.row()
            tbody += row

            for key, _ in self.fields.items():
                entry = nodes.entry()
                content = str(cat[key])

                if key == "id":
                    url = f"https://invent.kde.org/api/v4/projects/5927/jobs/artifacts/master/raw/out/{cat['filename']}?job=build-catlogs"
                    content = f"<a href='{url}'>{content}</a>"

                if content != "":
                    f = HTMLFilter()
                    f.feed(content)
                    entry += f.get_tag()
                row += entry

        return [table]


def setup(app):
    app.add_directive("catalog-table", CatalogtableDirective)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
