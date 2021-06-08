#ifndef __RFID_ERR_H__
#define __RFID_ERR_H__

#define RFID_LIB_SCOPE                    0x00001000

#define RFID_OK                           (RFID_LIB_SCOPE | 0x01 )
#define RFID_ERR_OTHER                    (RFID_LIB_SCOPE | 0x02 )
#define RFID_ERR_MEMORY_OVERRUN           (RFID_LIB_SCOPE | 0x03 )
#define RFID_ERR_MEMORY_LOCKED            (RFID_LIB_SCOPE | 0x04 )
#define RFID_ERR_INSUFFICIENT_POWER       (RFID_LIB_SCOPE | 0x05 )
#define RFID_ERR_NONSPECIFIC              (RFID_LIB_SCOPE | 0x06 )
#define RFID_ERR_NO_RESPONSE              (RFID_LIB_SCOPE | 0x07 )
#define RFID_ERR_PARSE                    (RFID_LIB_SCOPE | 0x08 )
#define RFID_ERR_REGULATION               (RFID_LIB_SCOPE | 0x09 )
#define RFID_ERR_ECHO_RESULT              (RFID_LIB_SCOPE | 0x0a )
#define RFID_ERR_INPUT                    (RFID_LIB_SCOPE | 0x0b )
#define RFID_ERR_SEND                     (RFID_LIB_SCOPE | 0x0c )
#define RFID_ERR_THREAD_CREATE            (RFID_LIB_SCOPE | 0x0d )
#define RFID_ERR_INVALID_HANDLE           (RFID_LIB_SCOPE | 0x0e )
#define RFID_ERR_BUFFER_OVERFLOW          (RFID_LIB_SCOPE | 0x0f )
#define RFID_ERR_FILE_NOT_EXIST           (RFID_LIB_SCOPE | 0x10 )
#define RFID_ERR_CMD_DEVICE_NOT_SUPPORT   (RFID_LIB_SCOPE | 0x11 )
#define RFID_ERR_WORDS_WRITE_ERR_Z        (RFID_LIB_SCOPE | 0x100)
#define RFID_ERR_WORDS_WRITE_ERR_3Z       (RFID_LIB_SCOPE | 0x200)



inline bool IS_OK (int n) { return n == RFID_OK; };
inline bool IS_ERRZ  (int n) { return (n & 0xFFFFFF00) == RFID_ERR_WORDS_WRITE_ERR_Z; }
inline bool IS_ERR3Z (int n) { return (n & 0xFFFFFF00) == RFID_ERR_WORDS_WRITE_ERR_3Z; }
inline int  ERRZ_VAL (int n) { return n & 0x000000FF; }
inline int  ERR3Z_VAL (int n) { return n & 0x000000FF; }


#endif
