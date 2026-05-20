import logging
import requests
import dataclasses
import os.path
import contextlib
from typing import Iterable, Any, Callable, TypeVar, Type, Generic, Union, Optional
from pathlib import Path

logger = logging.getLogger(__name__)


@dataclasses.dataclass
class DownloadData:
    """Holds information about downloadable data.  Usually the user
    supplies :class:`.filename` and :class:`.url` and the path is then
    supplied programatically by :func:`download` or
    :func:`lib.catalogfactory.Factory.download_cached`.
    """

    url: str  #: the url where the file shall be downloaded from
    filename: Optional[str] = None  #: the filename that the file shall be saved under
    _path: str = ""

    def __post_init__(self):
        if self.filename is None:
            self.filename = self.url.split("/")[-1]

    @property
    def path(self) -> str:
        """The path to the downloaded file.

        :raises RuntimeError: when the file does not exist (both at
                              assignment and at retrieval)
        """
        if self._path == "":
            raise RuntimeError(f"{self.url} is not yet downloaded.")

        return self._path

    @path.setter
    def path(self, value: str):
        """Assigns path to the downloaded file.

        :raises RuntimeError: when the file does not exist
        """
        if not os.path.isfile(value):
            raise RuntimeError(f"{value} does not exist.")

        self._path = value

    def open(self, *args, **kwargs):
        """Open the downloaded file (if it exists).

        Corresponds to the built-in :func:`io.open` with the first argument presupplied."""
        return open(self.path, *args, **kwargs)


def download(download_dir: str, download_data: Iterable[DownloadData]):
    """Downloads a file from :class:`DownloadData.url` for each member of
    ``download_data`` and saves it in the ``download_dir``.

    This function also sets :class:`DownloadData.path` to the downloaded file.
    """

    Path(download_dir).mkdir(parents=True, exist_ok=True)

    for data in download_data:
        filepath = os.path.join(download_dir, data.filename)

        logger.info(f"Downloading: {data.url}")

        if os.path.isfile(filepath):
            os.remove(filepath)

        with open(filepath, "wb") as f:
            req = requests.get(data.url, allow_redirects=True, stream=True)
            for part in req:
                f.write(part)

        data.path = filepath


T = TypeVar("T")


class ByteReader:
    """A utility class to read from binary files where each data field
    corresponds to a byte range in a line."""

    def __init__(self, line: bytes):
        """:param line: The line to be parsed."""
        self._line = line

    def get(
        self,
        begin: int,
        end: int,
        d_type: Callable[[bytes], Union[T, str]] = lambda b: b.decode("utf-8"),
    ) -> Union[T, str]:
        """Get data from the byte-range ``begin`` to ``end`` and convert it with ``d_type``.

        :param begin: beginning of the data range
        :param end: end of the data range
        :param d_type: A function that takes a bytestring and converts
                       it to some type ``T``, defaults to decoding as `utf-8` string.

        """
        return d_type(self._line[(begin - 1) : end].strip())
