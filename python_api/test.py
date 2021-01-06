from ctypes import *

class Foo():
    def __init__(self):
        print('__init__')
        self.obj = lib.Foo_new()
        print('self.obj={}'.format(self.obj))

    def bar(self):
        lib.Foo_bar(self.obj)

lib = cdll.LoadLibrary('/usr/local/lib/librfidmgr.so')
print('lib = {}'.format(lib))
f = Foo()
f.bar()

# test callback from libc qsort(),
# the last param of qsort() is a comparison function,
# here we provide a python function as the comparison function,
# Python calls C lib -> C lib calls Python callback function

CMPFUNC = CFUNCTYPE(c_int, POINTER(c_int), POINTER(c_int))
def py_comp_func(pa, pb):
    print("callback ({} {})".format(pa[0], pb[0]))
    return (pa[0] - pb[0])

cmp_func = CMPFUNC(py_comp_func)
libc = cdll.LoadLibrary('/lib/x86_64-linux-gnu/libc.so.6')
IntArray5 = c_int * 5
ia = IntArray5(5, 1, 7, 33, 99)
qsort = libc.qsort
qsort(ia, len(ia), sizeof(c_int), cmp_func)

for i in ia:
    print(i, end=" ")
print()
