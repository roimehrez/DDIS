import h5py
import numpy


def load_file(name,dataset):
    f = h5py.File(name)
    return numpy.array(f[dataset])
