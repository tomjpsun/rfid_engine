#ifndef __RFID_ERR_H__
#define __RFID_ERR_H__


#define RFID_OK                           0x1
#define RFID_ERR_OTHER                    0x2
#define RFID_ERR_MEMORY_OVERRUN           0x3
#define RFID_ERR_MEMORY_LOCKED            0x4
#define RFID_ERR_INSUFFICIENT_POWER       0x5
#define RFID_ERR_NONSPECIFIC              0x6
#define RFID_ERR_WORDS_WRITE_ERR_Z      0x100
#define RFID_ERR_WORDS_WRITE_ERR_3Z     0x200



inline bool IS_OK (int n) { return n == RFID_OK; };
inline bool IS_ERRZ  (int n) { return (n & 0x00000F00) == RFID_ERR_WORDS_WRITE_ERR_Z; }
inline bool IS_ERR3Z (int n) { return (n & 0x00000F00) == RFID_ERR_WORDS_WRITE_ERR_3Z; }
inline int  ERRZ_VAL (int n) { return n & 0x000000FF; }
inline int  ERR3Z_VAL (int n) { return n & 0x000000FF; }


#endif
