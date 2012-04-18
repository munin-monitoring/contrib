from distutils.core import setup
from pypmmn.pypmmn import __version__

PACKAGE = "pypmmn"
NAME = "pypmmn"
DESCRIPTION = "Python port of the 'Poor man's munin-node'"
AUTHOR = "Michel Albert"
AUTHOR_EMAIL = "michel@albert.lu"

setup(
    name=NAME,
    version=__version__,
    description=DESCRIPTION,
    long_description=open("README.rst").read(),
    author=AUTHOR,
    author_email=AUTHOR_EMAIL,
    license="BSD",
    url='https://github.com/exhuma/munin-contrib/tree/master/tools/pypmmn',
    packages=['pypmmn'],
    scripts=['pypmmn/pypmmn.py'],
)

