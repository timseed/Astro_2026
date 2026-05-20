.. _cli:

Command Line Tool
=================

The central entry-point for interacting with the catalog repository is
the cli-tool `pykstars`.

Installation
------------

.. note:: The tool is not meant to be used outside the catalog
          repository. Therefore installing it in a ``venv`` or in
          docker is recommended. Make sure to use the ``--editable``
          flag when installing with pip.

Bare Metal
~~~~~~~~~~
0. Clone `the repo <https://invent.kde.org/vboettcher/kstars-catalogs>`_.
1. Install the ``pykstars`` module. Instructions can be found `here <https://invent.kde.org/vboettcher/kstars/-/blob/dso_overhaul/kstars/python/readme.md>`_.
2. Run ``pip install --editable .`` in the root of the repository (potentially in a ``venv``).

Docker
~~~~~~

There exists a docker-image ``hiro98/pykstars:latest``  that has the pykstars module pre-installed.
Running

.. code-block:: shell

   docker run -v $(pwd):/stuff --rm -it -w /stuff hiro98/pykstars:latest bash -c 'pip install -e . && bash'

should drop you in a shell with the ``kscat`` command available.

Usage
-----

Building Catalogs
~~~~~~~~~~~~~~~~~

Building catalogs is accomplished with the ``build`` subcommand.  The
whole catalog collection can be build and deduped as well as just a
sub-collection.  The extreme case is of course building just a single
catalog, for example when developing a new catalog.

.. note:: Parsed catalogs will be cached. This cache is dropped
          automatically when a catalog file is edited, but when in
          doubt it is better to clean the cache manually.

.. click:: main:build
   :prog: kscat build
   :nested: full

Cleaning the Cache
~~~~~~~~~~~~~~~~~~

.. click:: main:clean
   :prog: kscat clean
   :nested: full

Listing available Catalogs
~~~~~~~~~~~~~~~~~~~~~~~~~~

This command may be used to either print a summary table of available
catalogs (the default behavior) or print a detailed information about
a single catalog.

For mode detailed inspection, an ``sqlite`` viewer is recommended.

.. click:: main:list_catalogs
   :prog: kscat list-catalogs
   :nested: full
