"""A random catalog generator to test the catalog factory"""

from lib.catalogfactory import Factory, Catalog
from lib.utility import DownloadData
from . import open_ngc
import pandas
import math
import zipfile
import os.path
from astropy.coordinates import Angle
from astropy import units as u
from pykstars import ObjectType

TYPE_MAP = {  # see skyobject.h, enum TYPE and
    # http://www.klima-luft.de/steinicke/ngcic/rev2000/Explan.htm#3.3
    1: ObjectType.GALAXY,
    2: ObjectType.GASEOUS_NEBULA,  # Supernova remnants / dark nebulae are disambiguated later
    3: ObjectType.PLANETARY_NEBULA,
    4: ObjectType.OPEN_CLUSTER,
    5: ObjectType.GLOBULAR_CLUSTER,
    6: ObjectType.TYPE_UNKNOWN,  # Most likely a HII region, but there are
    # exceptions (e.g. NGC 206), so we defer to
    # unknown.
    9: ObjectType.CATALOG_STAR,
}


class NGCICSteinicke(Factory):
    meta = Catalog(
        id=2,
        name="NGC IC (Steinicke)",
        author="Dr. Wolfgang Steinicke",
        license="Free for non-commercial use only",
        maintainer="Valentin Boettcher <hiro@protagon.space>",
        description="""The Revised New General Catalogue and Index Catalogue contains and
describes all objects from the original NGC/IC, compiled by
Dreyer. Additionally, many new objects (named with extension letters)
or companions are included, so the catalogues list alltogether 13957
entries.""",
        source="<a href='http://www.klima-luft.de/steinicke/ngcic/ngcic_e.htm'>Dr. Wolfgang Steinicke</a>",
        precedence=0.1,
        version=6,
        colors={
            "default": "#00fff2",
            "night.colors": "#ff6164",
            "moonless-night.colors": "#ff6164",
        },
        image="steinicke.gif",
    )

    def get_data(self):
        ngc = DownloadData(
            filename="NI2023.zip",
            url="http://www.klima-luft.de/steinicke/ngcic/rev2000/NI2023.zip",
        )

        self.download_cached(ngc)

        with zipfile.ZipFile(
            ngc.path,
            "r",
        ) as zip_ref:
            zip_ref.extractall(self._download_dir)

    def load_objects(self):
        catalog = pandas.read_excel(self._in_download_dir("NI2023.xls"))

        # For field descriptions, see:
        # http://www.klima-luft.de/steinicke/ngcic/rev2000/Explan.htm

        for _, obj in catalog.iterrows():
            if obj["S"] in [
                7,
                8,
                10,
            ]:  # N.B. We wish to include HII region etc. designations (e.g. NGC604)
                continue  # duplicate or compound entries

            name = (
                ("NGC " if obj["N"] == "N" else "IC ")
                + str(obj["NI"])
                + (str(obj["A"]) if not pandas.isnull(obj["A"]) else "")
                + ("-" + str(obj["C"]) if not pandas.isnull(obj["C"]) else "")
            )

            subtype = obj["TYPE"]
            type_ = TYPE_MAP[obj["S"]]
            if type_ == ObjectType.CATALOG_STAR:
                if subtype != "*":
                    type_ = (
                        ObjectType.ASTERISM
                    )  # Lump multiple stars and asterisms into asterisms
            elif type_ == ObjectType.GASEOUS_NEBULA:
                if subtype == "SNR":
                    type_ = ObjectType.SUPERNOVA_REMNANT
                elif subtype == "DN":
                    type_ = ObjectType.DARK_NEBULA
                # FIXME: KStars does not have subtypes for emission and reflection nebulae

            ids = []

            for i in range(1, 12):
                id_ = str(obj[f"ID{i}"])

                if id_.startswith("M ") or id_.startswith("IC "):
                    ids.append(name)
                    name = id_
                elif id_ != "nan":
                    ids.append(id_)

            assert obj["V"] in ("+", "-"), obj["V"]
            dec_sign = 1 if obj["V"] == "+" else -1
            cat_obj = self._make_catalog_object(
                type=type_,
                ra=Angle(
                    f"{obj['RH']}h {obj['RM']}m {obj['RS']}s", unit="hourangle"
                ).degree,
                dec=Angle(
                    f"{dec_sign*obj['DG']}d {obj['DM']}m {obj['DS']}s",
                    unit=u.degree,
                ).degree,
                magnitude=(
                    obj["VMAG"] if not pandas.isnull(obj["VMAG"]) else float("nan")
                ),
                name=name,
                long_name=(", ".join(ids) if ids else ""),
                catalog_identifier=str(obj["NI"]),
                major_axis=obj["X"],
                minor_axis=obj["Y"],
            )

            if not pandas.isnull(obj["PA"]):
                cat_obj.position_angle = obj["PA"]

            yield cat_obj

    def get_dublicates(self, query_fn, catalogs):
        open_ngc_id = open_ngc.OpenNGC.meta.id
        if open_ngc_id not in catalogs:
            return []

        for obj in query_fn(self.meta.id):
            suspects = query_fn(
                open_ngc_id, f"trixel = {obj.trixel} AND name = '{obj.name}'", 1
            )

            if len(suspects) == 0:
                suspects = query_fn(
                    open_ngc_id,
                    f"trixel = {obj.trixel} AND long_name LIKE '%{obj.name}%'",
                    1,
                )

            for suspect in suspects:
                if suspect.trixel == obj.trixel:
                    yield {(self.meta.id, obj.hash), (open_ngc_id, suspects[0].hash)}
