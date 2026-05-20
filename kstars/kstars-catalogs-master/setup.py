from setuptools import setup

setup(
    name="KStars Catalog Tool",
    version="0.0.2",
    py_modules=["main"],
    install_requires=[
        "Click>=8.0.1",
        "pandas>=2.0.0",
        "astropy>=4.2",
        "astroquery>=0.4.2",
        "wget>=3.2",
        "lxml>=4.6.3",
        "xlrd>=2.0.1",
        "pykstars",
    ],
    entry_points={
        "console_scripts": [
            "kscat = main:cli",
        ],
    },
)
