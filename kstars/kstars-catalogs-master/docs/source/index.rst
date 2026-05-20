KStars Catalogs
===============

The here documented tooling is a framework for generating new ``Deep
Sky Object`` catalogs for `kstars <https://edu.kde.org/kstars/>`_. The
basic idea is that the individual catalogs are implemented as python
modules which perform data acquisition, parsing and deduplication in
stages. Thus this tooling is meant to be run in CI and regenerate all
catalogs each time, although stages may be skipped and/or cached.

.. toctree::
   :maxdepth: 2
   :caption: Contents

   pages/how-it-works.rst
   pages/cli.rst
   pages/catalogs.rst
   pages/apidoc.rst

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
