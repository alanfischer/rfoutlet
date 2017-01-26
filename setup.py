"""A setuptools based setup module.
See:
https://packaging.python.org/en/latest/distributing.html
https://bitbucket.com/pallindo/rfoutlet
"""

from setuptools import setup, Extension
from codecs import open
from os import path

here = path.abspath(path.dirname(__file__))

with open(path.join(here, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

module = Extension('rfoutlet',
                    sources = [
                        'source/RFOutlet.cpp',
                    ])

setup(
    name='pyrfoutlet',
    version='0.0.1',
    description='A library for controlling Holiday Time rf outlets',
    long_description=long_description,
    url='https://bitbucket.com/pallindo/rfoutlet',
    author='Alan Fischer',
    author_email='alan@lightningtoads.com',
    license='MIT',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Topic :: Home Automation',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
    ],
    keywords='rf outlet',
    py_modules = ["pyrfoutlet"],
    ext_modules = [module]
)
