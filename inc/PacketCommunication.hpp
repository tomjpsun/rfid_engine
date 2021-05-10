#ifndef _PACKET_COMMUNICATION_HPP_
#define _PACKET_COMMUNICATION_HPP_

/* PacketCommunication.h: Davicom RF communication header for ESL system.       */
/*                                                                              */
/*==============================================================================*/
/* Header Name  : PacketCommunication.h                                         */
/* Purpose      : RFID communication                                              */
/* Autor        : Richard Chuang                                                */
/* Email        : richard_chung@davicom.com.tw                                  */
/* Description  :                                                               */
/* Reference    :                                                               */
/* Copyright    : (c) Davicom Semiconductor Inc. All Rights Reserved.           */
/* -----------------------------------------------------------------------------*/
/* Version      : 1.0.0.0                                                       */
/* Update       : 2020-11-02                                                    */
/* Modified     : Richard Chung                                                 */
/* Description  :                                                               */
/* -----------------------------------------------------------------------------*/
/* How to use:                                                                  */
/*                                                                              */


//==============================================================================
//--- Declarative Device Identifier  ---
//==============================================================================
//------------------------------------------------------------------------------
//--- ID Field Definition ---
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//--- Packet Type  ---
//------------------------------------------------------------------------------
/* RF Packet Type */

#define RF_PT_REQ_GET_FIRMWARE_VERSION				        0x00000001	// V: display reader firmware version
#define RF_PT_REQ_GET_READER_ID						0x00000002	// S : display reader ID
#define RF_PT_REQ_GET_SIGNLE_TAG_EPC_ONCE			        0x00000003	// Q : Read single tag EPC
#define RF_PT_REQ_GET_TAG_MEMORY_DATA				        0x00000004	// R : read tag memory data
#define RF_PT_REQ_SET_TAG_MEMORY_DATA				        0x00000005	// W : write data to tag memory
#define RF_PT_REQ_KILL_TAG						0x00000006	// K : kill tag
#define RF_PT_REQ_LOCK_TAG_MEMORY					0x00000007	// L : lock memory
#define RF_PT_REQ_SET_ACCESS_PASSWORD				        0x00000008	// P : set ACCESS password for ACCESS operation
#define RF_PT_REQ_GET_MULTI_TAG_EPC     			        0x00000009	// U : Read multiple tag EPC / Multi - TAG read EPC
#define RF_PT_REQ_LOGGING_COMMAND_OPERATION			        0x0000000A	// G : logging command operation
#define RF_PT_REQ_SELECT						0x0000000B	// T : Select
#define RF_PT_REQ_GET_POWER_LEVEL					0x0000000C	// N0 : Read RF power level
#define RF_PT_REQ_SET_POWER_LEVEL					0x0000000D	// N1 : Set RF power level
#define RF_PT_REQ_GET_REGULATION					0x0000000E	// N4 : Read RF Regulation
#define RF_PT_REQ_SET_REGULATION					0x0000000F	// N5 : Set RF Regulation
#define RF_PT_REQ_GET_MULTI_EPC_AND_MEMORY_DATA 	                0x00000010	// UR : Multi - TAG read EPC and read tag memory data
#define RF_PT_REQ_GET_TAG_EPC_ID_AND_MEMORY_DATA	                0x00000011	// QR : Display tag EPC ID and read tag memory data

#define RF_PT_REQ_GET_SIGNLE_ANTENNA				        0x00000012	// @Antenna : Get Single Antenna (@Antenna)
#define RF_PT_REQ_SET_SIGNLE_ANTENNA				        0x00000013	// @Antenna : Set Single Antenna (@Antenna[N])
#define RF_PT_REQ_GET_LOOP_ANTENNA					0x00000014	// @LoopAntenna : Get Loop Antenna (@LoopAntenna)
#define RF_PT_REQ_SET_LOOP_ANTENNA					0x00000015	// @LoopAntenna : Set Loop Antenna (@LoopAntenna[N])
#define RF_PT_REQ_GET_LOOP_TIME						0x00000016	// @LoopTime : Get Loop Antenna (@LoopTime)
#define RF_PT_REQ_SET_LOOP_TIME						0x00000017	// @LoopTime : Set Loop Antenna (@LoopTime[N])
#define RF_PT_REQ_GET_DATE_TIME						0x00000018	// Get Date (@SETDATE)
#define RF_PT_REQ_SET_DATE_TIME						0x00000019	// Set Date (@SETDATE%04d%02d%02d%02d%02d%0

#define RF_PT_REQ_GET_TAG_BANK						0x0000001A	// R : read tag memory data
#define RF_PT_REQ_SET_TAG_BANK						0x0000001B	// W : write data to tag memory
#define RF_PT_REQ_GET_TAG_EPC						0x0000001C	// R : read tag memory data
#define RF_PT_REQ_SET_TAG_EPC						0x0000001D	// W : write data to tag memory
#define RF_PT_REQ_GET_TAG_TID						0x0000001E	// R : read tag memory data
#define RF_PT_REQ_SET_TAG_TID						0x0000001F	// W : write data to tag memory
#define RF_PT_REQ_GET_TAG_USER						0x00000020	// R : read tag memory data
#define RF_PT_REQ_SET_TAG_USER						0x00000021	// W : write data to tag memory
#define RF_PT_REQ_GET_TAG_EPC_TID					0x00000022	// QR : display tag EPC ID and TID
#define RF_PT_REQ_GET_TAG_EPC_USER					0x00000023	// QR : display tag EPC ID and TID
#define RF_PT_REQ_GET_SIGNLE_TAG_EPC_LOOP			        0x00000024	// @Q : Read single tag EPC
#define RF_PT_REQ_GET_MULTI_TAG_EPC_LOOP			        0x00000025	// @U : Read multiple tag EPC / Multi - TAG read EPC

#define RF_PT_REQ_GET_MODULE_VERSION					0x00000026	// Display reader mdoule version (@Version)
#define RF_PT_REQ_INVENTORY_TAG_EPC_TID_ONCE		        	0x00000027	// U[X],R : Read multiple tag EPC and TID
#define RF_PT_REQ_INVENTORY_TAG_EPC_TID_LOOP		        	0x00000028	// @U[X] : Read multiple tag EPC and TID
#define RF_PT_REQ_OPEN_HEARTBEAT					0x00000029	// @HeartbeatTime : Open Heartbeat (@HeartbeatTime[N])
#define RF_PT_REQ_CLOSE_HEARTBEAT					0x0000002A	// @HeartbeatTime : Get Heartbeat (@HeartbeatTime)
#define RF_PT_REQ_GET_TAG_BANK_LOOP					0x0000002B	// @R : read bank loop
#define RF_PT_REQ_GET_MULTI_BANK_LOOP			                0x0000002C	// @U,R : Read multiple tag Bank loop
#define RF_PT_REQ_GET_MULTI_BANK_ONCE			                0x0000002D	// U,R : Read multiple tag Bank loop

//--- Response --
#define RF_PT_RES_GET_FIRMWARE_VERSION					0x00000081	// V: display reader firmware version
#define RF_PT_RES_GET_READER_ID						0x00000082	// S : display reader ID

#define RF_PT_RES_GET_TAG_MEMORY_DATA				        0x00000084	// R : read tag memory data
#define RF_PT_RES_SET_TAG_MEMORY_DATA				        0x00000085	// W : write data to tag memory
#define RF_PT_RES_KILL_TAG						0x00000086	// K : kill tag
#define RF_PT_RES_LOCK_TAG_MEMORY					0x00000087	// L : lock memory
#define RF_PT_RES_SET_ACCESS_PASSWORD				        0x00000088	// P : set ACCESS password for ACCESS operation
#define RF_PT_RES_GET_MULTI_TAG_EPC					0x00000089	// U : Read single tag EPC / Multi - TAG re
#define RF_PT_RES_LOGGING_COMMAND_OPERATION			        0x0000008A	// G : logging command operation
#define RF_PT_RES_SET_SESSION						0x0000008B	// T : Set Session
#define RF_PT_RES_GET_POWER_LEVEL					0x0000008C	// N0 / N1 : Read / Set RF power level
#define RF_PT_RES_SET_POWER_LEVEL					0x0000008D	// N0 / N1 : Read / Set RF power level
#define RF_PT_RES_GET_REGULATION					0x0000008E	// N4 / N5 : Read / Set RF Regulation
#define RF_PT_RES_SET_REGULATION					0x0000008F	// N4 / N5 : Read / Set RF Regulation
#define RF_PT_RES_GET_MULTI_EPC_ID_ADN_MEMORY_DATA              	0x00000090	// UR : Multi - TAG read EPC and read tag memory data
#define RF_PT_RES_GET_TAG_EPC_ID_AND_MEMORY_DATA	                0x00000091	// QR : Display tag EPC ID and read tag memory data
#define RF_PT_RES_GET_SIGNLE_ANTENNA				        0x00000092	// @Antenna : Get Single Antenna (@Antenna)
#define RF_PT_RES_SET_SIGNLE_ANTENNA				        0x00000093	// @Antenna : Set Single Antenna (@Antenna[N])
#define RF_PT_RES_GET_LOOP_ANTENNA					0x00000094	// @LoopAntenna : Get Loop Antenna (@LoopAntenna)
#define RF_PT_RES_SET_LOOP_ANTENNA					0x00000095	// @LoopAntenna : Set Loop Antenna (@LoopAntenna[N])
#define RF_PT_RES_GET_LOOP_TIME						0x00000096	// @LoopTime : Get Loop Antenna (@LoopTime)
#define RF_PT_RES_SET_LOOP_TIME						0x00000097	// @LoopTime : Set Loop Antenna (@LoopTime[N])
#define RF_PT_RES_GET_TAG_BANK						0x0000009A	// R : read tag memory data
#define RF_PT_RES_SET_TAG_BANK						0x0000009B	// W : write data to tag memory
#define RF_PT_RES_GET_TAG_EPC						0x0000009C	// R : read tag memory data
#define RF_PT_RES_SET_TAG_EPC						0x0000009D	// W : write data to tag memory
#define RF_PT_RES_GET_TAG_TID						0x0000009E	// R : read tag memory data
#define RF_PT_RES_SET_TAG_TID						0x0000009F	// W : write data to tag memory
#define RF_PT_RES_GET_TAG_USER						0x000000A0	// R : read tag memory data
#define RF_PT_RES_SET_TAG_USER						0x000000A1	// W : write data to tag memory
#define RF_PT_RES_GET_TAG_EPC_TID					0x000000A2	// QR : display tag EPC ID and TID
#define RF_PT_RES_GET_TAG_EPC_USER					0x000000A3	// QR : display tag EPC ID and TID

#define RF_PT_RES_GET_SIGNLE_TAG_EPC_LOOP			        0x000000A4	// @Q : Read single tag EPC
#define RF_PT_RES_GET_MULTI_TAG_EPC_LOOP			        0x000000A5	// @U : Read single tag EPC / Multi - TAG read EPC
#define RF_PT_RES_GET_MODULE_VERSION				        0x000000A6	// Display reader mdoule version (@Version)

#define RF_PT_RES_INVENTORY_TAG_EPC_TID_ONCE		                0x000000A7	// U[X],R : Read multiple tag EPC and TID
#define RF_PT_RES_INVENTORY_TAG_EPC_TID_LOOP		                0x000000A8	// @U[X] : Read multiple tag EPC and TID

#define RF_PT_RES_OPEN_HEARTBEAT					0x000000A9	// @HeartbeatTime : Open Heartbeat (@HeartbeatTime[N])
#define RF_PT_RES_CLOSE_HEARTBEAT					0x000000AA	// @HeartbeatTime : Get Heartbeat (@HeartbeatTime)

//------------------------------------------------------------------------------
//--- Packet String  ---
//------------------------------------------------------------------------------

#define RF_PS_RES_LOOP_END							"\n@END\r\n"

//------------------------------------------------------------------------------
//--- Antenna  ---
//------------------------------------------------------------------------------
// Antenna|  1 |  2 |  3 |  4 |
// -------+----+----+----+----+
//       1| 01 | 09 | 17 | 25 |
//       2| 02 | 10 | 18 | 26 |
//       3| 03 | 11 | 19 | 27 |
//       4| 04 | 12 | 20 | 28 |
//       5| 05 | 13 | 21 | 29 |
//       6| 06 | 14 | 22 | 30 |
//       7| 07 | 15 | 23 | 31 |
//       8| 08 | 16 | 24 | 32 |
//
// Ex. Set Loop Antenna: 1,2,9,17,18,19,31,32
// int[] bAnt = new int[4];
// bAnt[0] = 1 + 2;		// Antenna1 + Antenna2
// bAnt[1] = 1;			// Antenna9
// bAnt[2] = 1 + 2 + 4;	// Antenna17 + Antenna18 + Antenna19
// bAnt[3] = 64 + 128;  // Antenna31 + Antenna32

// Ex. Set Loop Antenna: 1,2,11,12,16,17,21,22,25,31,32
// bAnt[0] = 1 + 2;			// 0x03: Antenna1 + Antenna2
// bAnt[1] = 4 + 8 + 128;	// 0x8C: Antenna11 + Antenna12 + Antenna16
// bAnt[2] = 1 + 32 + 64;	// 0x31: Antenna17 + Antenna21 + Antenna22
// bAnt[3] = 1+ 64 + 128;	// 0xC1: Antenna25 + Antenna31 + Antenna32
// Send: <LF>@LoopAntenna01000100<CR>  <== 0x0A 0x40 0x4C 0x6F 0x6F 0x70 0x41 0x6e 0x74 0x65 0x6E 0x6E 0x61
//                         C1318C03        0x43 0x31 0x33 0x31 0x38 0x43 0x30 0x33 0x0D
//                         c1318c03        0x63 0x31 0x33 0x31 0x38 0x63 0x30 0x33 0x0D

// Ex. Set Loop Antenna: 9,25
//
// Send: <LF>@LoopAntenna01000100<CR>		<== 0x0A 0x40 0x4C 0x6F 0x6F 0x70 0x41 0x6e 0x74 0x65 0x6E 0x6E 0x61 0x30 0x31 0x30 0x30 0x30 0x31 0x30 0x30 0x30 0x0d
// Recv: <LF>@LoopAntenna01000100<CR><LF>	<== 0x0A 0x40 0x4C 0x6F 0x6F 0x70 0x41 0x6e 0x74 0x65 0x6E 0x6E 0x61 0x30 0x31 0x30 0x30 0x30 0x31 0x30 0x30 0x30 0x0d 0x0a

#define RF_ANTENNA_01								0x00000001	// Antenna1-1
#define RF_ANTENNA_02								0x00000002	// Antenna1-2
#define RF_ANTENNA_03								0x00000004	// Antenna1-3
#define RF_ANTENNA_04								0x00000008	// Antenna1-4
#define RF_ANTENNA_05								0x00000010	// Antenna1-5
#define RF_ANTENNA_06								0x00000020	// Antenna1-6
#define RF_ANTENNA_07								0x00000040	// Antenna1-7
#define RF_ANTENNA_08								0x00000080	// Antenna1-8
#define RF_ANTENNA_09								0x00000100	// Antenna2-1
#define RF_ANTENNA_10								0x00000200	// Antenna2-2
#define RF_ANTENNA_11								0x00000400	// Antenna2-3
#define RF_ANTENNA_12								0x00000800	// Antenna2-4
#define RF_ANTENNA_13								0x00001000	// Antenna2-5
#define RF_ANTENNA_14								0x00002000	// Antenna2-6
#define RF_ANTENNA_15								0x00004000	// Antenna2-7
#define RF_ANTENNA_16								0x00008000	// Antenna2-8
#define RF_ANTENNA_17								0x00010000	// Antenna3-1
#define RF_ANTENNA_18   							0x00020000	// Antenna3-2
#define RF_ANTENNA_19								0x00040000	// Antenna3-3
#define RF_ANTENNA_20								0x00080000	// Antenna3-4
#define RF_ANTENNA_21								0x00100000	// Antenna3-5
#define RF_ANTENNA_22								0x00200000	// Antenna3-6
#define RF_ANTENNA_23								0x00400000	// Antenna3-7
#define RF_ANTENNA_24								0x00800000	// Antenna3-8
#define RF_ANTENNA_25								0x01000000	// Antenna4-1
#define RF_ANTENNA_26								0x02000000	// Antenna4-2
#define RF_ANTENNA_27								0x04000000	// Antenna4-3
#define RF_ANTENNA_28   							0x08000000	// Antenna4-4
#define RF_ANTENNA_29								0x10000000	// Antenna4-5
#define RF_ANTENNA_30								0x20000000	// Antenna4-6
#define RF_ANTENNA_31								0x40000000	// Antenna4-7
#define RF_ANTENNA_32								0x80000000	// Antenna4-8

#endif // _PACKET_COMMUNICATION_HPP_
