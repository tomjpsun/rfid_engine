from ctypes import *

lib = cdll.LoadLibrary('/usr/local/lib/librfidmgr.so')
print('lib = {}'.format(lib))


def errcheck(result, func, args):
    print( "result = {}".format(result) )
    #if result != 0:
    #    raise Exception
    return 0


args_table = [
    (lib.RfidOpen, [c_char_p, c_char, c_int]),
    (lib.RFInventoryEPC, [c_int, c_int, c_bool, POINTER(POINTER(c_char)), POINTER(c_int)]),
    (lib.RFClose, [c_int])
    ]

class Foo():
    def __init__(self, ip, port, ip_type = 1):
        print('__init__')

        # check with args table
        for (foo, larg) in args_table:
            foo.argtypes = larg
            foo.errcheck = errcheck

        # open a connection, save the opened handle
        ip_bytes = ip.encode('utf-8')
        self.handle = lib.RfidOpen(c_char_p(ip_bytes), c_char(ip_type), c_int(port));
        print('self.handle={}'.format(self.handle))


    def __del__(self):
        lib.RFClose(self.handle)

    # return JSON string
    def InventoryEPC(self, slot, loop):
        _slot = c_int(slot)
        _loop = c_bool(loop)
        _json_str = POINTER(c_char)()
        _len = c_int()
        lib.RFInventoryEPC (self.handle, _slot, _loop, byref(_json_str) , byref(_len))
        json_str = str()
        for i in range(0, _len.value):
            json_str += _json_str[i].decode('utf-8')
        return json_str

f = Foo("192.168.88.91", 1001)
json_result = f.InventoryEPC( 3, False )
print(json_result)
