from ctypes import *

lib = cdll.LoadLibrary('/usr/local/lib/librfidmgr.so')
print('lib = {}'.format(lib))


def errcheck(result, func, args):
    print( "result = {}".format(result) )
    #if result != 0:
    #    raise Exception
    return 0


args_table = [
    (lib.RFOpen, [c_int]),
    (lib.RFInventoryEPC, [c_int, c_int, c_bool, POINTER(POINTER(c_char)), POINTER(c_int)]),
    (lib.RFClose, [c_int])
    ]

class Foo():
    def __init__(self, index):
        print('__init__')

        # check with args table
        for (foo, larg) in args_table:
            foo.argtypes = larg
            foo.errcheck = errcheck

        # open a connection, save the opened handle
        self.handle = lib.RFOpen(c_int(index));


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


    def SingleCommand(self, userCmd):
        _userCmd = userCmd.encode('utf-8')
        _userCmdLen = len(_userCmd)
        _json_str = POINTER(c_char)()
        _len = c_int()
        lib.RFSingleCommand (self.handle, c_char_p(_userCmd), c_int(_userCmdLen), byref(_json_str) , byref(_len))
        json_str = str()
        for i in range(0, _len.value):
            json_str += _json_str[i].decode('utf-8')
        return json_str


    def ReadMultibank(self, slot, loop, bank, start, wordLen):
        _slot = c_int(slot)
        _loop = c_bool(loop)
        _bank = c_int(bank)
        _start = c_int(start)
        _wordLen = c_int(wordLen)
        _json_str = POINTER(c_char)()
        _len = c_int()
        lib.RFReadMultiBank (self.handle, _slot, _loop, _bank,
                             _start, _wordLen, byref(_json_str) , byref(_len))

        json_str = str()
        for i in range(0, _len.value):
            json_str += _json_str[i].decode('utf-8')
        return json_str

    def Close(self):
        lib.RFClose(self.handle)

if __name__ == '__main__':

    # Open handle
    f = Foo(0)

    json_result = f.InventoryEPC( 3, False )
    print("InventoryEPC result = {}".format(json_result))

    json_result = f.ReadMultibank( 3, True, 1, 0, 6)
    print("ReadMultibank result = {}".format(json_result))

    json_result = f.SingleCommand("U3")
    print("SingleCommand result = {}".format(json_result))

    # close handle
    f.Close()
