from ctypes import *

lib = cdll.LoadLibrary('/usr/local/lib/librfidmgr.so')
print('lib = {}'.format(lib))

CALLBACK_T = CFUNCTYPE(None, POINTER(c_char), c_size_t)

class Foo():
    def __init__(self):
        print('__init__')
        self.obj = lib.Foo_new()
        print('self.obj={}'.format(self.obj))

    def bar(self):
        lib.Foo_bar(self.obj)

    def get_antenna_data(self, cb_func):
        lib.Foo_get_antenna_data(self.obj, cb_func)

    def get_coordinate(self, cb_func):
        lib.Foo_get_coordinate(self.obj, cb_func)

    def get_statistics(self, cb_func):
        lib.Foo_get_statistics(self.obj, cb_func)


# antenna_callback(antenna_data, size):
#
#     antenna_data is a ctypes.LP_c_char object,
#         use array operator[] to extract its content to bytearray, and
#         decode bytearray to python string
#     size is the bytes count in antenna_data

def antenna_callback(antenna_data, size):
    print("get antenna_data({}) = ".format(size))
    antenna_str = (antenna_data[:size]).decode("utf-8")
    print(antenna_str)


# coordinate_callback(coordinate_data, size):
#
#     coordinate_data is a ctypes.LP_c_char object,
#         use array operator[] to extract its content to bytearray, and
#         decode bytearray to python string
#     size is the bytes count in antenna_data

def coordinate_callback(coordinate_data, size):
    print("get coordinate_data({}) = ".format(size))
    coordinate_str = (coordinate_data[:size]).decode("utf-8")
    print(coordinate_str)


# statistics_callback(statistics_data, size):
#
#     statistics_data is a ctypes.LP_c_char object,
#         use array operator[] to extract its content to bytearray, and
#         decode bytearray to python string
#     size is the bytes count in antenna_data

def statistics_callback(statistics_data, size):
    print("get statistics_data({}) = ".format(size))
    statistics_str = (statistics_data[:size]).decode("utf-8")
    print(statistics_str)


# make python function to c type function
antenna_cb = CALLBACK_T(antenna_callback)
coordinate_cb = CALLBACK_T(coordinate_callback)
statistics_cb = CALLBACK_T(statistics_callback)

f = Foo()
f.get_antenna_data(antenna_cb)
f.get_coordinate(coordinate_cb)
f.get_statistics(statistics_cb)
