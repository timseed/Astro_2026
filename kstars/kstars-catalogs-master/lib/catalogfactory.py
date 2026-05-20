"""The basic plumbing and template for creating custom catalogs."""
from .catalogsdb import Catalog, CatalogObject
from typing import (
    Iterator,
    Optional,
    List,
    Set,
    Callable,
    Union,
    Tuple,
    Dict,
    ClassVar,
    Any,
)
from pathlib import Path
import re
import pickle
import inspect
import logging
import os.path
from . import utility
from .dupe_set import DupeSet


def catalog_dirname(meta: Catalog):
    return (
        f"{meta.id}_"
        + "".join(e for e in re.sub(r"\s+", "_", meta.name) if e.isalnum())
        + f"_{meta.version}"
    )


class Factory:
    """The base class for all catalogs.

    :param cache_dir: the path to the cache directory
    :param data_dir: the path to the data directory

    Each catalog subclasses this class.  The class variable
    :attr:`.meta` is overwritten by a catalog to specify catalog
    metadata.

    **Because of caching, the modifications to the state of the class
    instance is not persistent. Therefore the** :attr:`._state` **, which
    is persisted by pickling should be used.**

    These methods are commonly overwritten:

    - The method :meth:`.__post_init__` may be overwritten to perform
      initialization as an alternative to the constructor.

    - The method :meth:`.get_data` may be overwritten to acquire the catalog data.
    - The method :meth:`.load_objects` may be overwritten to retrieve objects from the catalog data.
    - The method :meth:`.get_dublicates` may be overwritten to identify dublicates between catalogs.

    To overwrite those methods is less common:

    - The method :meth:`.is_fresh` may be overwritten to customize the detection of an outdated cache.

    """

    meta: ClassVar[Catalog]  #: holds metadata about a catalog

    ###############################################################################
    #                                  Interface                                  #
    ###############################################################################

    def __post_init__(self, *args, **kwargs) -> None:
        """This method can be overwritten by custom catalogs and gets called
        after the initialization."""
        pass

    def get_data(self) -> None:
        """This method can be overwritten to download catalog data into
        :meth:`._download_dir`.

        See :class:`lib.utility.DownloadData` and :meth:`.download_cached`.
        """
        pass

    def load_objects(self) -> Iterator[CatalogObject]:
        """A generator that yields :class:`lib.catalogsdb.CatalogObject` to be
        inserted into the catalog database.

        Should use :meth:`._make_catalog_object` for convenience.
        """
        pass

    def get_dublicates(
        self,
        query_fn: Callable[[Union[str, int], Optional[str], int], List[CatalogObject]],
        catalogs: Dict[int, Catalog],
    ) -> Iterator[DupeSet]:
        """Returns an iterator of sets of catalog-id and object hash
        pairs. Those sets signify the dublicate objects. The catalog
        database can be queried by the query_fn which is basically
        :meth:`lib.catalogsdb.CatalogsDB.retrieve_objects`. ``catalogs``
        is a dict of catalog id and meta information.

        """
        return iter([])

    ###########################################################################
    #                                 Utility                                 #
    ###########################################################################

    def __init__(self, cache_dir: str, data_dir: str) -> None:
        # meta has to be set!
        if self.meta is None:
            raise ValueError("The ``meta`` class variable of the catalog must be set!")

        self._cache_dir = os.path.join(cache_dir, self.dirname)
        Path(self._cache_dir).mkdir(parents=True, exist_ok=True)

        self._data_dir = os.path.join(data_dir, self.__module__.split(".")[-1])
        self.logger = logging.getLogger(self.meta.name)

        self._state = None  #: Storage for persistend state. Is being synced to disk.
        self.__post_init__()

        if self.is_fresh(self.state_cache_filename):
            with open(self.state_cache_filename, "rb") as state:
                self._state = pickle.load(state)

    def __del__(self):
        if self._state is None:
            return

        with open(self.state_cache_filename, "wb") as state:
            pickle.dump(self._state, state, protocol=pickle.HIGHEST_PROTOCOL)

    @property
    def _download_dir(self):
        """Path to the download directory for the catalog."""
        return os.path.join(self._cache_dir, "Downloads")

    def _in_download_dir(self, path: str) -> str:
        """Transforms a path relative to the download
        directory to an absolute path.
        """
        return os.path.join(self._download_dir, path)

    def _in_data_dir(self, path: str) -> str:
        """Transforms a path relative to the data
        directory to an absolute path.
        """
        return os.path.join(self._data_dir, path)

    def _in_cache_dir(self, path: str) -> str:
        """Transforms a path relative to the cache
        directory to an absolute path.
        """
        return os.path.join(self._cache_dir, path)

    def _make_catalog_object(self, *args, **kwargs):
        """Constructs a :class:`lib.catalogsdb.CatalogObject` with the field
        ``catalog`` prebound to the current catalog. All other arguments
        are passed through to the constructor.
        """
        return CatalogObject(*args, catalog=self.meta.id, **kwargs)

    @property
    def dirname(self) -> str:
        """The filename for the generated catalog. (without extension) Used to
        label the cache directory."""
        return catalog_dirname(self.meta)

    @property
    def filename(self) -> str:
        """The filename for the generated catalog. (with extension)"""
        return self.dirname + ".kscat"

    @property
    def catalog_cache_filename(self) -> str:
        """The path where the catalog is dumped for caching."""

        return self._in_cache_dir(self.filename)

    @property
    def state_cache_filename(self) -> str:
        """The directory where the state of the catalog class instance is cached."""
        return self._in_cache_dir(self.dirname + ".pickle")

    def is_fresh(self, path: str) -> bool:
        """Returns wether the file under ``path`` is to considered fresh to be
        loaded from cache.

        The standard implementation checks if the file's m-time is
        more or as recent as the m-time of the module file.
        """

        if not os.path.isfile(path):
            return False

        class_file = inspect.getfile(self.__class__)
        return os.path.getmtime(class_file) < os.path.getctime(path)

    def download_cached(self, *download_data: utility.DownloadData):
        """Download a file and cache it. Uses :meth:`.is_fresh`."""

        download_dir = self._download_dir  # has to be computed (is property)
        filtered_downloads = filter(
            lambda dat: not self.is_fresh(self._in_download_dir(dat.filename)),
            download_data,
        )

        utility.download(download_dir, filtered_downloads)
        for data in download_data:
            data.path = self._in_download_dir(data.filename)
