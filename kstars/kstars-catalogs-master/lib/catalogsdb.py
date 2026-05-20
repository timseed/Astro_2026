"""Code to create and manipulate a kstars catalog database."""

import logging
import sqlite3
import pykstars
from pykstars import sqlstatements as s
from dataclasses import dataclass, field, asdict
from datetime import datetime
from types import SimpleNamespace
import dataclasses
from typing import (
    Callable,
    Sequence,
    TypeVar,
    Dict,
    Union,
    Optional,
    List,
    MutableMapping,
    Tuple,
    Set,
    Any,
    Iterator,
    ClassVar,
)
import re

logger = logging.getLogger(__name__)


def row_to_catalogobject(row: sqlite3.Row):
    obj = CatalogObject()
    fields = [a for a in dir(obj) if not a.startswith("__")]

    for field_ in fields:
        if row[field_]:
            setattr(obj, field_, row[field_])

    return obj


@dataclass
class CatalogObject:
    """A DSO in the catalog. Just a simple container. No magic!

    :param id: oid  of the object
    :param oid: oid  of the object
    :param typed: Type of object
    :param ra: Right Ascension
    :param dec: Declination
    :param magnitude: magnitude (brightness)
    :param name: Primary name
    :param long_name: Long name (common name)
    :param catalog_identifier: a catalog specific identifier
    :param catalog: catalog id
    :param major_axis: major axis (arcminutes)
    :param minor_axis: minor axis (arcminutes)
    :param position_angle: position angle (degrees)
    :param flux: integrated flux (optional)

    """

    trixel: int = 0
    hash: bytes = bytes()
    oid: bytes = bytes()

    def set_type(self, tp: Union[int, pykstars.ObjectType, property]):
        if isinstance(tp, int):
            self.__dict__["type"] = tp
            return

        if isinstance(tp, property):
            self.__dict__["type"] = tp.fget(self)
            return

        self.__dict__["type"] = tp.value

    def get_type(self) -> int:
        return self.__dict__.get("type")  # type: ignore

    type: Union[int, pykstars.ObjectType] = property(get_type, set_type)  # type: ignore
    ra: float = 0
    dec: float = 0
    magnitude: Optional[float] = None
    name: str = ""
    long_name: str = ""
    catalog_identifier: str = ""
    major_axis: Optional[float] = None
    minor_axis: Optional[float] = None
    position_angle: Optional[float] = None
    flux: Optional[float] = None
    catalog: int = 0

    del set_type, get_type


@dataclass
class Catalog:
    """Catalog Metadata."""

    #: The numeric and *unique* id of the catalog. Should be chosen
    #: greater that the biggest previusly given id.
    id: int

    #: The name of the catalog.
    name: str

    #: The precedence of the catalog. If an dublicate object exists in
    #: another catalog with a higher precedence, it will shadow an
    #: object from this catalog. Should be between 0 and 1.
    precedence: float = 0

    #: The author of the catalog. Is set to the maintainer if not
    #: specified.
    author: str = ""

    #: The source of the catalog. (url, book, etc...)
    source: str = ""

    #: A (brief) description. HTML allowed.
    description: str = ""

    #: Version of the catalog. To be incremented for each content
    #: update
    version: int = 0

    #: Wether the catalog is mutable. This is irrelevant for catalog
    #: creation.
    mut: bool = True

    #: Whether the catalog is enabled. This is irrelevant for catalog
    #: creation.
    enabled: bool = True

    _default_color: ClassVar[str] = "#4DA6FF"

    colors: Dict[str, str] = field(
        default_factory=lambda: {"default": Catalog._default_color}
    )
    """
    The color that the catalog's objects are being rendered in as a dict of the form::

       {
           'default': '#[hex rgb color code for default color]',
           '[color scheme file name]': '#[hex rgb color code]'
       }

    The default color is used when the color for a specific color scheme is not specified.
    """

    @property
    def color(self) -> str:
        """The color string that will be saved into the database."""

        color_string = self.colors.get("default", Catalog._default_color)

        for scheme, color in self.colors.items():
            if scheme == "default":
                continue

            color_string += f";{scheme};{color}"

        return color_string

    #: The path of the preview image of the catalog relative to the
    #: data directory.
    image: Optional[str] = None

    #: License of the catalog.
    license: str = ""

    #: Maintainer, style: "Name <email>"
    maintainer: str = ""

    #: Timestamp (the build time). Will be set by the build process.
    timestamp: datetime = datetime.now()

    @property
    def maintainer_email(self) -> Optional[str]:
        "The email address of the maintainer."
        match = re.search(r"<(.*?)>", self.maintainer)
        if match:
            return match.group(1)

        return None

    @property
    def maintainer_name(self) -> str:
        "The email address of the maintainer."
        match = re.search(r"^(.*?)\s*<", self.maintainer)

        if match:
            return match.group(1)

        return self.maintainer

    def __post_init__(self):
        self.enabled = self.enabled != 0
        self.mut = self.mut != 0

        maintainer = self.maintainer

        if self.license == "":
            logger.warn(f"Catalog '{self.name}' has an empty license.")

        if self.author == "" and self.maintainer != "":
            self.author = self.maintainer

    def to_dict(self) -> dict[str, "any"]:
        dct = asdict(self)
        dct["color"] = self.color

        return dct


class CATALOGS(SimpleNamespace):
    """An Enumeration of hardcoded catalog names."""

    master = s.master_catalog
    all_objects = s.all_catalog_view


class CatalogsDB:
    """A utility to manipulate a catalog db in the kstars format.

    Wraps and augments the kstars python bindings and is usually not
    required for catalog packaging.
    """

    def __init__(self, db_filename: str, htmesh_level: int = 3) -> None:
        self._db_filename = db_filename
        self._db = sqlite3.connect(self._db_filename)
        self._db.row_factory = sqlite3.Row
        self._htmesh_level = htmesh_level
        self._indexer = pykstars.Indexer(self._htmesh_level)
        self._manager = pykstars.DBManager(self._db_filename)

    @property
    def filename(self):
        """Returns the filename of the underlying db."""
        return self._db_filename

    def insert_object(self, obj: CatalogObject) -> None:
        """Insert an object into a catalog. Regenerates its ``hash``, ``trixel`` and also
        its ``oid`` if it is empty.
        """

        self._insert_objects(self._update_objects(iter([obj])))

    def insert_objects(self, objs: Iterator[CatalogObject]) -> None:
        """Insert objects into a catalog. Regenerates their ``hash``, ``trixel`` and also
        their ``oid`` if it is empty.
        """

        self._insert_objects(self._update_objects(objs))

    def _update_objects(
        self, objects: Iterator[CatalogObject]
    ) -> Iterator[CatalogObject]:
        """Regenerates ``obj``'s ``hash``, ``trixel`` and also
        id: bytes = bytes(its ``oid`` if it is empty.
        """

        for obj in objects:
            obj.hash = pykstars.get_id(asdict(obj))
            obj.trixel = self._indexer.get_trixel(obj.ra, obj.dec)
            if len(obj.oid) == 0:
                obj.oid = obj.hash

            yield obj

    def _insert_objects(self, objects: Iterator[CatalogObject]) -> None:
        """Insert the given ``objects`` into their catalogs."""
        cursor = self._db.cursor()

        cursor.execute("BEGIN TRANSACTION")
        for obj in objects:
            cursor.execute(s.insert_dso(obj.catalog), asdict(obj))

        cursor.execute("END TRANSACTION")
        self._db.commit()

    def get_catalog(self, catalog_id: int) -> Optional[Catalog]:
        """Load catalog metadata from the database."""
        cursor = self._db.cursor()
        cursor.execute(s.get_catalog_by_id, {"id": catalog_id})

        cat = cursor.fetchone()
        return Catalog(**cat) if cat is not None else None

    def register_catalog(self, cat: Catalog) -> None:
        """Add a catalog ``cat`` to the database."""
        if cat.id < 0:
            raise ValueError(f"Catalog id={cat.id} is invalid!")

        cat.mut = False
        cat.enabled = True

        raise_if_not_successful(self._manager.register_catalog(cat.to_dict()))

    def update_catalog_meta(self, cat: Catalog) -> None:
        """Update ``cat`` in the database."""
        if cat.id < 0:
            raise ValueError(f"Catalog id={cat.id} is invalid!")

        cat.mut = False
        cat.enabled = True

        raise_if_not_successful(self._manager.update_catalog_meta(cat.to_dict()))

    def compile_master_catalog(self) -> None:
        if not (
            self._manager.update_catalog_views()
            and self._manager.compile_master_catalog()
        ):
            raise RuntimeError("Could not compile the master catalog.")

    def remove_catalog(self, id: int) -> None:
        raise_if_not_successful(self._manager.remove_catalog(id))

    def retrieve_objects(
        self, catalog: Union[str, int], query: Optional[str] = None, limit: int = -1
    ) -> List[CatalogObject]:
        """Retrieve objects from the ``catalog`` (either id or table
        name). ``query`` is inserted as WHERE clause verbatim.
        The result list size is limited to ``limit``.

        To search all catalogs, use
        :attr:`lib.catalogsdb.CATALOGS.all_objects` first argument.
        """
        if isinstance(catalog, int):
            catalog = f"cat_{catalog}"

        query_str = (
            f"SELECT * FROM {catalog}"
            + (f" WHERE {query}" if query is not None else "")
            + f" LIMIT {limit}"
        )

        cursor = self._db.cursor()
        res = cursor.execute(query_str)
        return [row_to_catalogobject(row) for row in res]

    def get_object(self, catalog: int, hash: bytes):
        query_str = f"SELECT * FROM cat_{catalog} WHERE hash = ? LIMIT 1"

        cursor = self._db.cursor()
        res = cursor.execute(query_str, (hash,)).fetchone()

        return row_to_catalogobject(res) if res is not None else None

    def dump_catalog(self, catalog_id: int, file_path: str) -> None:
        raise_if_not_successful(self._manager.dump_catalog(catalog_id, file_path))

    def import_catalog(self, file_path: str, overwrite: bool) -> None:
        raise_if_not_successful(self._manager.import_catalog(file_path, overwrite))

    def process_dupes(
        self, dupes: Set[Tuple[int, bytes]], precedence_ordered: List[int]
    ):
        """Set the object ids of the object to the same value deterministically."""
        fiducial_hash = min(dupes, key=lambda dupe: precedence_ordered.index(dupe[0]))[
            1
        ]

        cursor = self._db.cursor()
        for catalog, hash in dupes:
            query_str = f"UPDATE cat_{catalog} SET oid = ? WHERE hash = ?"
            cursor.execute(query_str, (fiducial_hash, hash))
        self._db.commit()


###############################################################################
#                                   Typevars                                  #
###############################################################################

KeyType = TypeVar("KeyType")
T = TypeVar("T")

###############################################################################
#                                   Utility                                   #
###############################################################################


def partition(
    lst: Sequence[T], key_fn: Callable[[T], KeyType]
) -> Dict[KeyType, List[T]]:
    """Partition ``lst`` by the key determined by ``key``."""
    partitioned: Dict[KeyType, List[T]] = {}

    for elem in lst:
        key = key_fn(elem)
        if key not in partitioned:
            partitioned[key] = []

        partitioned[key].append(elem)

    return partitioned


def strip_None(dct: MutableMapping[Any, Any]) -> MutableMapping[Any, Any]:
    """Strip any entries with ``None`` values from ``dct``."""
    return {key: val for (key, val) in dct.items() if val is not None}


def raise_if_not_successful(success: Tuple[bool, str]) -> None:
    """Raise a runtime error if the first entry of ``success`` is ``False``."""
    if not success[0]:
        raise RuntimeError(success[1])
