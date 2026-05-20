"""
The OpenNGC catalog for kstars.

Maintained by: Christian Dersch <lupinix at mailbox dot org>
Catalog data by: Mattia Verga <mattia dot verga at tiscali dot it>

"""

from lib.catalogfactory import Factory, Catalog
from lib.utility import DownloadData
import zipfile
import os
from astropy.io import ascii
import numpy as np
import glob
from astropy.coordinates import Angle
from astropy import units as u
from pykstars import ObjectType


def getnum(ngcstr):
    # Function to extract the integer number of an object,
    # returns 0 for non-main objects to avoid duplicates
    if ngcstr[0] == "N":
        return ngcstr[3:].lstrip("0")
    elif ngcstr[0] == "I":
        return ngcstr[2:].lstrip("0")
    else:
        return "0"


def readable_names(ngcstr):
    """Generate better readable names, e.g. NGC 224 instead of NGC0224."""
    if ngcstr[0] == "N":
        return "NGC " + ngcstr[3:].lstrip("0")
    elif ngcstr[0] == "I":
        return "IC " + ngcstr[2:].lstrip("0")
    else:
        # Change nothing if don't know what to do
        return ngcstr


def map_classification(classification):
    """
    Mapping KStars ngcic.dat → OpenNGC

    OpenNGC      KStars ngcic     Type

    *                1            Star
    **               17           Double Star
    *Ass             13           Association of stars (13 matches better than 5)
    Ocl              3            Open Cluster
    Gcl              4            Globular Cluster
    Cl+N             5            Star cluster + Nebula
    G                8            Galaxy
    GPair            8            Galaxy Pair
    GTrpl            8            Galaxy Triplet
    GGroup           8            Group of galaxies
    PN               6            Planetary Nebula
    HII              5            HII Ionized region
    EmN              5            Emission Nebula
    Neb              5            Nebula
    RfN              5            Reflection Nebula
    SNR              7            Supernova remnant
    Nova                          Nova star
    NonEx                         Nonexistent object
    Dup                           Duplicate
    Other                         Other
    """
    mapping = {
        "*": ObjectType.STAR,
        "**": ObjectType.MULT_STAR,
        "*Ass": ObjectType.ASTERISM,
        "OCl": ObjectType.OPEN_CLUSTER,
        "GCl": ObjectType.GLOBULAR_CLUSTER,
        "Cl+N": ObjectType.GASEOUS_NEBULA,
        "G": ObjectType.GALAXY,
        "GPair": ObjectType.GALAXY,
        "GTrpl": ObjectType.GALAXY,
        "GGroup": ObjectType.GALAXY_CLUSTER,
        "PN": ObjectType.PLANETARY_NEBULA,
        "HII": ObjectType.GASEOUS_NEBULA,
        "EmN": ObjectType.GASEOUS_NEBULA,
        "Neb": ObjectType.GASEOUS_NEBULA,
        "RfN": ObjectType.GASEOUS_NEBULA,
        "SNR": ObjectType.SUPERNOVA_REMNANT,
    }
    return mapping.get(classification, ObjectType.TYPE_UNKNOWN)


def extract_pgcnum(id_list):
    if id_list != "--":
        s = id_list.split(",")
        for id in s:
            if id[0:3] == "PGC":
                pgc = id[3:].strip().lstrip("0")
                if len(pgc) <= 6:
                    return "PGC " + pgc
        return None
    else:
        return None


def extract_ugcnum(id_list):
    if id_list != "--":
        s = id_list.split(",")
        for id in s:
            if id[0:3] == "UGC":
                return "UGC " + id[3:].strip().lstrip("0")
        return None
    else:
        return None


def messier_str(messier):
    if messier != "--":
        return "M " + str(messier)
    else:
        return None


# Vectorize functions for string reformat operations to run them on whole table
getnum_vect = np.vectorize(getnum)
map_classification_vect = np.vectorize(map_classification)


class OpenNGC(Factory):
    meta = Catalog(
        id=1,
        name="OpenNGC",
        author="Mattia Verga (mattia dot verga at tiscali dot it)",
        description="""OpenNGC is a database containing positions and main data of NGC (New General Catalogue) and IC (Index Catalogue) objects. Unlike other similar databases which are released with license limitations, OpenNGC is released under CC-BY-SA-4.0 license, which allows the use for a wider range of cases.""",
        source="<a href='https://github.com/mattiaverga/OpenNGC/releases/tag/v20230218'>OpenNGC Github Release</a>",
        license="CC-BY-SA-4.0",
        maintainer="Christian Dersch <chrisdersch@gmail.com>",
        precedence=1,
        version=9,
        image="ngc.jpg",
        colors={
            "default": "#4DA6FF",
            "night.colors": "#ff6164",
            "moonless-night.colors": "#ff6164",
        },
    )

    def get_data(self):
        ngc = DownloadData(
            filename="NGC.zip",
            url="https://github.com/mattiaverga/OpenNGC/archive/refs/tags/v20231203.zip",
        )

        self.download_cached(ngc)

        with zipfile.ZipFile(
            ngc.path,
            "r",
        ) as zip_ref:
            zip_ref.extractall(self._download_dir)

    def load_objects(self):
        ngc_full = ascii.read(
            glob.glob(self._in_download_dir("OpenNGC*/database_files/NGC.csv"))[0],
            delimiter=";",
        )
        ngc = ngc_full[getnum_vect(ngc_full["Name"]) != "0"]

        for obj in ngc:
            mag = None
            for mag_name in ["B", "V", "J", "H", "K", "Cstar B", "Cstar U", "Cstar V"]:
                c_mag = obj[mag_name + "-Mag"]
                if c_mag != "--":
                    mag = c_mag
                    break

            if mag is None:
                mag = float("nan")  # make it very faint

            catalog_obj = self._make_catalog_object(
                name=readable_names(obj["Name"]),
                ra=Angle(obj["RA"], unit="hourangle").degree,
                dec=Angle(obj["Dec"], unit=u.degree).degree,
                magnitude=float(mag),
                type=map_classification(obj["Type"]),
                catalog_identifier=obj["Name"],
            )

            if obj["MajAx"] != "--":
                catalog_obj.major_axis = obj["MajAx"]

            if obj["MinAx"] != "--":
                catalog_obj.minor_axis = obj["MinAx"]

            if obj["PosAng"] != "--":
                catalog_obj.position_angle = float(obj["PosAng"])

            messier = messier_str(obj["M"])
            other_ids = [
                extract_pgcnum(obj["Identifiers"]),
                extract_ugcnum(obj["Identifiers"]),
            ]

            if messier is not None:
                other_ids.append(catalog_obj.name)
                catalog_obj.name = messier

            if obj["Name"] == "NGC5866":
                # Workaround for M 102 which is referenced to NGC 5866
                # nowadays, we want the user to be able to search for
                # M 102 although not in catalog
                other_ids.append("M 102")

            other_ids = [id_ for id_ in other_ids if id_ is not None]

            common = obj["Common names"]
            if common != "--":
                other_ids = common.split(",") + other_ids

            catalog_obj.long_name = ", ".join(other_ids) if other_ids else ""

            yield catalog_obj

        # Add M40 and M45 manually
        yield self._make_catalog_object(
            type=ObjectType.MULT_STAR,
            ra=Angle("12:22:12.5", unit="hourangle").degree,
            dec=Angle("+58:4:59", unit=u.degree).degree,
            magnitude=9.7,
            major_axis=49,
            name="M 40",
            long_name="Winnecke 4",
            catalog_identifier="M 40",
        )

        yield self._make_catalog_object(
            type=ObjectType.OPEN_CLUSTER,
            ra=Angle("03:47:24", unit="hourangle").degree,
            dec=Angle("+24:07:00", unit=u.degree).degree,
            major_axis=110,
            magnitude=1.6,
            name=f"M 45",
            long_name="Pleiades",
            catalog_identifier="M 45",
        )
