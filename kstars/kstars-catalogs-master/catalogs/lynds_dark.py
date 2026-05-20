"""The Lynds' Catalogue of Dark Nebulae (LDN) straight from
https://cdsarc.unistra.fr/viz-bin/cat/VII/7A
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
import math
from pykstars import ObjectType


class LyndsDark(Factory):
    meta = Catalog(
        id=6,
        name="Lynds' Catalogue of Dark Nebulae",
        maintainer="Jasem Mutlaq <mutlaqja@ikarustech.com>",
        license="Free for non-commercial and/or educational use",
        author="Lynds B.T.",
        description="""This catalog is an updated version from the published version. The
catalog was based on the red and blue prints of the National
Geographic - Palomar Observatory Sky Atlas. The catalog contains
positions, both equatorial and galactic for the centers of dark
nebulae.""",
        source="<a href='https://cdsarc.unistra.fr/viz-bin/cat/VII/7A'>CDS</a>",
        precedence=1,
        version=1,
        image="ldn.png",
        colors={"default": "#d3d3d3"},
    )

    def __post_init__(self):
        self.ldn = DownloadData(
            filename="ldn.dat",
            url="http://cdsarc.unistra.fr/ftp/VII/7A/ldn",
        )

    def get_data(self):
        self.download_cached(self.ldn)

    def load_objects(self):
        with self.ldn.open("rb") as cat:
            frame = FK5(equinox=Time("J1950"))
            fk5_2000 = FK5(equinox=Time(2000, format="jyear"))

            for line in cat.readlines():
                reader = ByteReader(line)
                cat_nr = reader.get(51, 54)
                coords = SkyCoord(
                    ra=(reader.get(6, 7, int) + reader.get(9, 12, float) * 1 / 60)
                    * u.hourangle,
                    dec=(
                        (-1 if reader.get(16, 16) == "-" else 1)
                        * (
                            reader.get(17, 18, int) * u.degree
                            + reader.get(20, 21, int) * u.arcmin
                        )
                    ),
                    frame=frame,
                )

                coords.transform_to(fk5_2000)
                radius = math.sqrt(reader.get(37, 43, float) / math.pi) * 60  # arcmin

                name = f"LDN {reader.get(1, 4)}"

                yield self._make_catalog_object(
                    type=ObjectType.DARK_NEBULA,
                    ra=coords.ra.degree,
                    dec=coords.dec.degree,
                    name=name,
                    major_axis=radius / 2,
                    minor_axis=radius / 2,
                    catalog_identifier=cat_nr,
                )
