from lib.catalogfactory import Factory, Catalog
from lib.catalogsdb import CATALOGS
import pandas as pa
import os.path
import pdb
from astropy import units as u
from astropy.coordinates import Angle
from . import open_ngc, ngcic_steinicke
from pykstars import ObjectType


class AbellPlanetaryNebulae(Factory):
    meta = Catalog(
        id=3,
        name="Abell Planetary Nebulae",
        author="George O. Abell",
        description="""The Abell Catalog of Planetary Nebulae was created in 1966 by George O. Abell and was composed of 86 entries thought to be planetary nebulae that were collected from discoveries, about half by Albert George Wilson and the rest by Abell, Robert George Harrington, and Rudolph Minkowski. All were discovered before August 1955 as part of the National Geographic Society – Palomar Observatory Sky Survey on photographic plates created with the 48-inch (1.2 m) Samuel Oschin telescope at Mount Palomar.""",
        source="<a href='https://astronomy-mall.com/Adventures.In.Deep.Space/abellcat.htm'>Adventures in Deep Space</a>",
        precedence=0.3,
        version=1,
        maintainer="Carl Knight <SleeplessAtKnight@gmail.com>",
        license="Free for non-commercial use only",
        colors={"default": "#dbffd5"},
        image="abell.jpg",
    )

    def load_objects(self):
        catalog = pa.read_html(self._in_data_dir("abellcat.htm"), header=0)[0]
        catalog = catalog[catalog["PNG# / OTHER"] != "See Notes"]
        self._state = catalog

        for _, obj in self._state.iterrows():
            radius = float(obj["SIZE"]) / 2
            yield self._make_catalog_object(
                type=ObjectType.PLANETARY_NEBULA,
                ra=Angle(obj["RA"], unit="hourangle").degree,
                dec=Angle(obj["DEC"], unit=u.degree).degree,
                magnitude=float(obj["MAG1"].rstrip("p")),
                name=f"Abell {obj['ABELL']}",
                catalog_identifier=str(obj["ABELL"]),
                major_axis=radius,
                minor_axis=radius,
            )

    def get_dublicates(self, query_fn, catalogs):
        if (
            open_ngc.OpenNGC.meta.id not in catalogs
            and ngcic_steinicke.NGCICSteinicke.meta.id not in catalogs
        ):
            return []

        for _, obj in self._state.iterrows():
            name: str = obj["PNG# / OTHER"]

            if name.startswith("NGC") or name.startswith("IC"):
                this_object = query_fn(
                    self.meta.id, f"catalog_identifier = {obj['ABELL']}"
                )

                if len(this_object) < 1:
                    continue

                this_object = this_object[0]

                suspects = query_fn(
                    CATALOGS.all_objects,
                    f"trixel = {this_object.trixel} AND name LIKE '{name}'",
                )

                if len(suspects) > 0:
                    yield {(self.meta.id, this_object.hash)}.union(
                        {(suspect.catalog, suspect.hash) for suspect in suspects}
                    )
