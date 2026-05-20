import logging
import shutil
import click
import builder
from typing import List, Optional
from multiprocessing import cpu_count

logger = logging.getLogger(__name__)


@click.group()
def cli():
    pass


COMMON_OPTIONS = [
    click.option(
        "--output-dir",
        "-o",
        default="out",
        type=click.Path(file_okay=False, writable=True, resolve_path=True),
        show_default=True,
        help="The directory in which the catalogs will be placed.",
    ),
    click.option(
        "--cache-dir",
        "-c",
        default=".cache",
        type=click.Path(file_okay=False, writable=True, resolve_path=True),
        show_default=True,
        help="The directory in which the catalogs will be placed.",
    ),
]


def add_options(options):
    def _add_options(func):
        for option in reversed(options):
            func = option(func)
        return func

    return _add_options


@cli.command()
@add_options(COMMON_OPTIONS)
@click.option(
    "--data-dir",
    "-d",
    default="data",
    type=click.Path(file_okay=False, writable=True, resolve_path=True),
    show_default=True,
    help="The directory of the static catalog data.",
)
@click.option(
    "--catalog",
    "-c",
    multiple=True,
    type=click.INT,
    help="""Catalog to be built.
Can be specified multiple times.
By default all catalogs are being built.""",
)
@click.option(
    "--nproc",
    "-n",
    default=cpu_count(),
    type=click.INT,
    help=f"""How many processes to use for parallel tasks.
The default is the number of processors ({cpu_count()}).""",
)
@click.option(
    "--knewstuff",
    default=None,
    type=click.STRING,
    help="""The prefix url for knewstuff. If specified the catalogs are output in a manner suitable for knewstuff in [output-dir]/knewstuff""",
)
def build(
    output_dir: str,
    cache_dir: str,
    data_dir: str,
    catalog: List[int],
    nproc: int,
    knewstuff: Optional[str],
):
    """Build kstars catalogs."""
    builder.build_catalogs(output_dir, cache_dir, data_dir, catalog, nproc, knewstuff)


@cli.command()
@add_options(COMMON_OPTIONS)
@click.option(
    "--cache-only",
    is_flag=True,
    default=False,
    type=click.BOOL,
    help="""Whether to clean only the cache""",
)
def clean(output_dir: str, cache_dir: str, cache_only: bool):
    """Clean cache and output."""
    dirs = [cache_dir] + ([] if cache_only else [output_dir])

    for directory in dirs:
        try:
            shutil.rmtree(directory)
            logger.info(f"Removed '{directory}'")
        except Exception:
            pass


@cli.command()
@click.option(
    "--html",
    "-h",
    multiple=False,
    default=False,
    is_flag=True,
    type=click.BOOL,
    help="""Output an HTML summary of the available catalogs.""",
)
@click.option(
    "--catalog",
    "-c",
    multiple=False,
    default=None,
    type=click.INT,
    help="""Print more information about the catalog with this id.""",
)
def list_catalogs(
    html: bool,
    catalog: Optional[int],
):
    """List all available catalogs."""
    if catalog is None:
        builder.list_catalogs(html)
    else:
        builder.print_catalog_info(catalog)


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    cli()
