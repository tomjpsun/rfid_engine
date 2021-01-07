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

def antenna_callback(antenna_data, size):
    print("get antenna_data({})".format(size))


antenna_cb = CALLBACK_T(antenna_callback)

f = Foo()
f.bar()
f.get_antenna_data(antenna_cb)
