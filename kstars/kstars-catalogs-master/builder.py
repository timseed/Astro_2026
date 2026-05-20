from lib import catalogsdb, catalogfactory, dupe_set
import inspect
import logging
import tempfile
import catalogs
from typing import Sequence, List, Optional, Iterator, Dict
from pathlib import Path
import click
import pandas
import os.path
from dataclasses import asdict
from multiprocessing import Pool
from contextlib import contextmanager
import shutil
import tarfile
import datetime
from lxml import etree

logger = logging.getLogger(__name__)

CATALOGS = [
    cat
    for (item, cat) in catalogs.__dict__.items()
    if not item.startswith("__")
    if hasattr(cat, "meta")
    and hasattr(cat, "__bases__")
    and catalogfactory.Factory in inspect.getmro(cat)
]


def get_catalogs_df() -> pandas.DataFrame:
    cats = []
    for catalog in CATALOGS:
        meta = dict(asdict(catalog.meta))
        meta["filename"] = catalogfactory.catalog_dirname(catalog.meta) + ".kscat"
        cats.append(meta)
    return pandas.DataFrame(cats)


def list_catalogs(html: bool) -> None:
    """List all available catalogs. Optionally as `html`"""
    df = get_catalogs_df()  # hack for printing tables

    if html:
        click.echo(
            df[["id", "name", "precedence", "description", "author", "source"]]
            .sort_values(by="id", ascending=True)
            .to_html(index=False, escape=False)
        )
    else:
        click.echo(
            df[["id", "name", "precedence"]]
            .sort_values(by="id", ascending=True)
            .to_string(index=False)
        )


def print_catalog_info(catalog_id: int) -> None:
    """Print the catalog metadata for the catalog with id `catalog_id`."""

    for catalog in CATALOGS:
        if catalog.meta.id == catalog_id:
            for key, val in asdict(catalog.meta).items():
                click.echo(f"{key}={val}")

            return


@contextmanager
def temp_db() -> Iterator[catalogsdb.CatalogsDB]:
    db_file = tempfile.NamedTemporaryFile()
    db = catalogsdb.CatalogsDB(db_file.name)

    yield db

    del db
    db_file.close()


def download_and_parse_stage(catalog: catalogfactory.Factory) -> catalogfactory.Factory:
    logger.info(f"Getting data for the catalog '{catalog.meta.name}'.")
    catalog.get_data()

    cache_path = catalog.catalog_cache_filename
    if os.path.isfile(cache_path) and catalog.is_fresh(cache_path):
        logger.info(f"Using '{catalog.meta.name}' from cache.")
    else:
        logger.info(
            f"Registering the catalog '{catalog.meta.name}' in a temporary db and parsing."
        )

        with temp_db() as db:
            db.register_catalog(catalog.meta)
            db.insert_objects(catalog.load_objects())
            db.dump_catalog(catalog.meta.id, catalog.catalog_cache_filename)

    return catalog


def get_dupes_from_catalog(
    catalog: catalogfactory.Factory, db_path: str, metas: Dict[int, catalogsdb.Catalog]
) -> List[dupe_set.DupeSet]:
    db = catalogsdb.CatalogsDB(db_path)
    return list(catalog.get_dublicates(db.retrieve_objects, metas))


def dedupe_stage(
    catalogs: List[catalogfactory.Factory], db: catalogsdb.CatalogsDB, nproc: int
) -> None:
    logger.info("Deduplicating.")

    container = dupe_set.DupeContainter()
    metas = {catalog.meta.id: catalog.meta for catalog in catalogs}

    tempfiles = [tempfile.NamedTemporaryFile() for _ in catalogs]
    for f in tempfiles:
        shutil.copy(db.filename, f.name)

    args = [(catalog, f.name, metas) for (catalog, f) in zip(catalogs, tempfiles)]

    with Pool(nproc) as p:
        dupes = p.starmap(get_dupes_from_catalog, args)

    for cat_dupes in dupes:
        container.add_many(cat_dupes)

    precedence_ordered = [
        catalog.meta.id
        for catalog in sorted(
            CATALOGS, key=lambda cat: (cat.meta.precedence, -cat.meta.id), reverse=True
        )
    ]

    for dupe in container.dupes:
        db.process_dupes(dupe, precedence_ordered)


def dump_stage(
    catalogs: List[catalogfactory.Factory],
    db: catalogsdb.CatalogsDB,
    out_dir: str,
    timestamp: datetime.datetime,
) -> None:
    """Dump the catalogs into individual files."""
    logger.info("Dumping the catalogs.")

    for catalog in catalogs:
        path = out_dir + "/" + catalog.filename
        logger.info(
            f"Dumping contents of the catalog '{catalog.meta.name}' into '{path}'."
        )
        catalog.meta.timestamp = timestamp
        db.update_catalog_meta(catalog.meta)
        db.dump_catalog(catalog.meta.id, path)


def build_catalogs(
    out_dir: str,
    cache_dir: str,
    data_dir: str,
    ids: Sequence[int],
    nproc: int,
    knewstuff: Optional[str],
) -> None:
    """Build catalogs. Either all catalogs or just the catalogs with an id
    in `ids`"""

    Path(out_dir).mkdir(parents=True, exist_ok=True)
    Path(cache_dir).mkdir(parents=True, exist_ok=True)

    enabled_catalogs = []
    loaded_ids = []
    for catalog_class in CATALOGS:
        catalog = catalog_class(cache_dir, data_dir)

        id = catalog.meta.id
        if len(ids) == 0 or id in ids:
            enabled_catalogs.append(catalog)

            if id in loaded_ids:
                raise RuntimeError(
                    f"""Could not initialize the
catalog '{catalog.meta.name}' The id {id} is already
in use!"""
                )

            loaded_ids.append(id)

    num_cats = len(enabled_catalogs)
    if num_cats < 1:
        raise RuntimeError("No catalogs to build!")

    nproc = min(num_cats, nproc)

    with Pool(nproc) as p:
        enabled_catalogs = p.map(download_and_parse_stage, enabled_catalogs)

    now = datetime.datetime.now()
    with temp_db() as db:
        logger.info
        for catalog in enabled_catalogs:
            logger.info(f"Loading the catalog '{catalog.meta.name}'.")
            db.import_catalog(catalog.catalog_cache_filename, True)

        db.compile_master_catalog()
        dedupe_stage(enabled_catalogs, db, nproc)

        db.compile_master_catalog()
        dump_stage(enabled_catalogs, db, out_dir, now)

    if knewstuff:
        package_catalogs(enabled_catalogs, out_dir, knewstuff)


def package_catalogs(catalogs: List[catalogfactory.Factory], out_dir: str, prefix: str):
    new_out_dir = os.path.join(out_dir, "knewstuff")
    Path(new_out_dir).mkdir(parents=True, exist_ok=True)

    stuff = etree.Element("knewstuff")

    for catalog in catalogs:
        path = os.path.join(out_dir, catalog.filename)
        tarpath = os.path.join(new_out_dir, catalog.filename) + ".tar.gz"

        with tarfile.open(tarpath, "w:gz") as tar:
            tar.add(path, arcname=catalog.filename)

        entry = etree.SubElement(stuff, "stuff")

        meta = catalog.meta
        etree.SubElement(entry, "name").text = meta.name
        etree.SubElement(entry, "type").text = "kstars/data"

        author = etree.SubElement(entry, "author")
        author.text = meta.maintainer_name

        # BUG: knewstuff can't handle emails apparaently
        # if meta.maintainer_email:
        #     author.set("email", meta.maintainer_email)

        etree.SubElement(entry, "license").text = meta.license
        summary = etree.SubElement(entry, "summary")
        summary.text = meta.description
        summary.set("lang", "en")

        etree.SubElement(entry, "version").text = str(meta.timestamp.timestamp())
        etree.SubElement(entry, "release").text = "1"
        etree.SubElement(entry, "releasedate").text = datetime.date.today().strftime(
            "%Y-%m-%d"
        )
        etree.SubElement(entry, "downloadsize1").text = str(
            round(os.path.getsize(tarpath) / (1024) ** 2, 1)
        )
        etree.SubElement(entry, "category").text = f"dso"
        etree.SubElement(entry, "id").text = str(meta.id)

        if meta.image:
            impath = os.path.join(new_out_dir, catalog.dirname)
            Path(impath).mkdir(parents=True, exist_ok=True)

            shutil.copyfile(
                catalog._in_data_dir(meta.image), os.path.join(impath, meta.image)
            )

            etree.SubElement(entry, "preview").text = os.path.join(
                prefix, catalog.dirname, meta.image
            )

        pl = etree.SubElement(entry, "payload")
        pl.text = os.path.join(prefix, catalog.filename + ".tar.gz")
        pl.set("lang", "en")

    xmlpath = os.path.join(new_out_dir, "dso.xml")
    logger.info(f"Writing knewstuff xml to: {xmlpath}")
    with open(xmlpath, "wb") as f:
        f.write(
            etree.tostring(
                stuff,
                encoding="UTF-8",
                xml_declaration=True,
                pretty_print=True,
                doctype='<!DOCTYPE knewstuff SYSTEM "knewstuff.dtd">',
            )
        )
