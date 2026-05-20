"""The hickson catalog parsed directly from
https://cdsarc.unistra.fr/viz-bin/cat/VII/213
"""

from lib.catalogfactory import Factory, Catalog
from lib.utility import DownloadData, ByteReader
from pykstars import ObjectType

import pickle
from astropy import units as u
from . import open_ngc, ngcic_steinicke
from astropy.time import Time
from astropy.coordinates import SkyCoord
from astropy.coordinates import FK5


class Hickson(Factory):
    meta = Catalog(
        id=5,
        name="Hickson Compact Groups",
        author="Hickson P.",
        maintainer="Akarsh Simha <akarsh@kde.org>",
        description="""The catalog of groups is a list of 100 compact groups of galaxies
identified by a systematic search of the Palomar Observatory Sky
Survey red prints. Each group contains four or more galaxies, has an
estimated mean surface brightness brighter than 26.0 magnitude per
arcsec2 and satisfies an isolation criterion.""",
        source="<a href='https://cdsarc.unistra.fr/viz-bin/cat/VII/213'>CDS</a>",
        precedence=0.2,
        version=1,
        license="Free for non-commercial and/or educational use",
        colors={"default": "#d7acff"},
        image="hickson.jpg",
    )

    def __post_init__(self):
        self.hick = DownloadData(
            url="http://cdsarc.unistra.fr/ftp/VII/213/groups.dat",
        )

        self._state = dict(names=dict())

    def get_data(self):
        self.download_cached(self.hick)

    def load_objects(self):
        with self.hick.open("rb") as cat:
            frame = FK5(equinox=Time(1950, format="jyear"))
            fk5_2000 = FK5(equinox=Time(2000, format="jyear"))

            for line in cat.readlines():
                reader = ByteReader(line)
                cat_nr = reader.get(1, 3)
                coords = SkyCoord(
                    ra=(
                        reader.get(5, 6, int)
                        + reader.get(7, 8, int) * 1 / 60
                        + reader.get(9, 10, int) * 1 / (60 ** 2)
                    )
                    * u.hourangle,
                    dec=(
                        (-1 if reader.get(11, 11) == "-" else 1)
                        * (
                            reader.get(12, 13, int) * u.degree
                            + reader.get(14, 15, int) * u.arcmin
                            + reader.get(16, 17, int) * u.arcsec
                        )
                    ),
                    frame=frame,
                )

                coords = coords.transform_to(fk5_2000)
                radius = reader.get(24, 28, float) / 2
                mag = reader.get(29, 33, float)

                names = [
                    name[1:]
                    for beg, end in [(46, 51), (53, 58), (60, 65), (67, 72)]
                    if len(name := reader.get(beg, end)) > 0 and name.startswith("N")
                ]

                if names:
                    self._state["names"][cat_nr] = names[0]

                name = f"Hickson {cat_nr}"
                yield self._make_catalog_object(
                    type=ObjectType.GALAXY_CLUSTER,
                    ra=coords.ra.degree,
                    dec=coords.dec.degree,
                    magnitude=mag,
                    name=name,
                    long_name=(f" (NGC {names[0]})" if names else ""),
                    major_axis=radius / 2,
                    minor_axis=radius / 2,
                    catalog_identifier=cat_nr,
                )

    def get_dublicates(self, query_fn, catalogs):
        open_ngc_id = open_ngc.OpenNGC.meta.id

        if open_ngc_id not in catalogs:
            return []

        for obj in query_fn(self.meta.id):
            if obj.catalog_identifier not in self._state["names"]:
                continue

            name = self._state["names"][obj.catalog_identifier]
            ngc_designation = "NGC" + name.zfill(4)

            suspects = query_fn(
                open_ngc_id,
                f"catalog_identifier LIKE '{ngc_designation}' AND trixel = {obj.trixel}",
            )

            for suspect in suspects:
                yield {(self.meta.id, obj.hash), (open_ngc_id, suspect.hash)}
