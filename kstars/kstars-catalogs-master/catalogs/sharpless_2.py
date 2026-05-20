"""The sharpless catalog parsed directly from
http://cdsarc.u-strasbg.fr/viz-bin/Cat?VII/20

Some guesses are being made about the magnitude onto which the
'brightness' values map.
"""

from lib.catalogfactory import Factory, Catalog
from lib.catalogsdb import CATALOGS
from lib.utility import DownloadData, ByteReader

import pandas as pd
import os.path
import pdb
from astropy import units as u
from astropy.coordinates import Angle
from . import open_ngc, ngcic_steinicke
import gzip
from astropy.time import Time
from astropy.coordinates import SkyCoord
from astropy.coordinates import FK5
from pykstars import ObjectType

map_brightness = {1: 11, 2: 10, 3: 9}  # rough guess


class Sharpless2(Factory):
    meta = Catalog(
        id=4,
        name="Sharpless HII region Catalog",
        author="Sharpless S.",
        maintainer="Jasem Mutlaq <mutlaqja@ikarustech.com>",
        license="Public Domain",
        description="""The catalogue describes the position, maximum angular diameters,
classifications according to form, structure and brightness, and the
number of associated stars. The acronym used in the literature to
designate objects from this catalogue is Sh 2 (e.g. Sh 2-1 for the
first HII region of the catalogue)""",
        source="<a href='http://cdsarc.u-strasbg.fr/viz-bin/Cat?VII/20'>CDS</a>",
        precedence=0.5,
        version=2,
        colors={"default": "#ffcddc"},
        image="sh2.jpg",
    )

    def __post_init__(self):
        self.sh_2 = DownloadData(
            filename="catalog.dat.gz",
            url="http://cdsarc.u-strasbg.fr/ftp/VII/20/catalog.dat.gz",
        )

    def get_data(self):
        self.download_cached(self.sh_2)

    def load_objects(self):
        with gzip.open(self.sh_2.path) as cat:
            frame = FK5(equinox=Time(1950, format="jyear"))
            fk5_2000 = FK5(equinox=Time(2000, format="jyear"))

            for line in cat.readlines():
                reader = ByteReader(line)
                cat_nr = reader.get(1, 4, int)
                coords = SkyCoord(
                    ra=(
                        reader.get(35, 36, int)
                        + reader.get(37, 38, int) * 1 / 60
                        + reader.get(39, 41, int) * 0.1 * 1 / (60 ** 2)
                    )
                    * u.hourangle,
                    dec=(
                        (-1 if reader.get(42, 42) == "-" else 1)
                        * (
                            reader.get(43, 44, int) * u.degree
                            + reader.get(45, 46, int) * u.arcmin
                            + reader.get(47, 48, int) * u.arcsec
                        )
                    ),
                    frame=frame,
                )

                coords = coords.transform_to(fk5_2000)

                radius = reader.get(49, 52, int) / 2

                yield self._make_catalog_object(
                    type=ObjectType.GASEOUS_NEBULA,
                    ra=coords.ra.degree,
                    dec=coords.dec.degree,
                    magnitude=map_brightness[reader.get(55, 55, int)],
                    name=f"Sh2 {cat_nr}",
                    long_name=f"Sharpless {cat_nr}",
                    major_axis=radius / 2,
                    minor_axis=radius / 2,
                    catalog_identifier=str(cat_nr),
                )
