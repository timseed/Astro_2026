from lib.catalogfactory import Factory, Catalog
from lib.catalogsdb import CATALOGS
import pandas as pa
import os.path
import pdb
from astropy import units as u
from astropy.coordinates import Angle
from . import open_ngc, ngcic_steinicke
from pykstars import ObjectType
import re


class Messier(Factory):
    meta = Catalog(
        id=13,
        name="Messier",
        author="Charles Messier and Pierre Méchain",
        description="""The Messier catalog is a renowned list of 110 deep-sky objects—galaxies, nebulae, and star clusters—compiled by 18th-century French astronomer Charles Messier to help comet hunters distinguish permanent "fuzzy" objects from comets. Published between 1771 and 1781, it features bright Northern Hemisphere objects (like M31, M42) ideal for amateur viewing. """,
        source="<a href=''https://en.wikipedia.org/wiki/Messier_object''</a>",
        precedence=0.4,
        version=1,
        maintainer="Tim Seed<tim@sy-edm.com>",
        license="Free for non-commercial use only",
        colors={
            "star-chart.colors": "#443322",
            "classic": "#443322",
            "default": "#4DA6FF",
            "night.colors": "#ff6164",
            "moonless-night.colors": "#ff6164",
        },
        image="messier.jpg",
    )
    records = []

    def load_objects(self):

        df = pa.read_csv(self._in_data_dir("Messier_from_pyongc.csv"), sep=",")

        self._state = df
        # for _, obj in self._state.iterrows():
        for i, (_, obj) in enumerate(self._state.iterrows()):
            yield self._make_catalog_object(
                type=int(obj["KStars_Type"]),
                ra=float(obj["ra_degrees"]),
                dec=float(obj["dec_degrees"]),
                magnitude=float(obj["vmag"]),
                catalog_identifier=str(obj["commonnames"]),
                name=str("M" + f"{obj['messier']:3d}"),
                major_axis=float(obj.majax),
                minor_axis=float(obj.minax),
            )

    def get_dublicates(self, query_fn, catalogs):
        if (
            open_ngc.OpenNGC.meta.id not in catalogs
            and ngcic_steinicke.NGCICSteinicke.meta.id not in catalogs
        ):
            return []

        for obj in self.records:
            name: obj.name

            if name.startswith("NGC") or name.startswith("IC"):
                ci = f"M{obj['messier']:3d}"
                this_object = query_fn(self.meta.id, f"catalog_identifier ={ci}")

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
