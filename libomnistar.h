/***************************************************************************
                        libomnistar.h  -  description
                            -------------------
  begin                : April 2005
  copyright            : (C) 2005 by Pierre Lamon
  email                : pierre.lamon@epfl.ch

  description          : commands for configuring the omnistar
                         WARNING! By now the lib only support one port for both configuration
                         and data acquisition
 ***************************************************************************/

#ifndef LIBOMNISTAR_H
#define LIBOMNISTAR_H

#include "libomnistar_struct.h"

#define OMNI_MAX_BUFF_SIZE 2048

/** Setup the communication with the satellite */
int  Omni_OpenPort(OMNI_COM_TYPE port, char *device, OMNI_BAUD_TYPE baudrate, int timeout);
int  Omni_ClosePort(OMNI_COM_TYPE port);
int  Omni_SetDefaultCmdPort(OMNI_COM_TYPE def_port);
int  Omni_AssignOmni(unsigned int freq, OMNI_BAUD_TYPE baud);
void Omni_InitDefaultComSetup(ComSetupStruct *com_str);
int  Omni_ComSetup(ComSetupStruct com_str);
int  Omni_LogSetup(LogSetupStruct log_str);
int  Omni_UnlogMsg(OMNI_SETUP_PORT_TYPE port, OMNI_MSG_TYPE msg);
int  Omni_UnlogAll(int check_reply);
int  Omni_EnableHP();
int  Omni_EnableVBS();
int  Omni_SaveConfig();
int  Omni_Reset(unsigned int seconds);
int  Omni_ParseStream(OMNI_COM_TYPE port, char *rec_buf, size_t buflen, TIMEVAL *stamp);
int  Omni_ParseOMNIHPPOS(HPPosStruct *hppos, const char *rec_buf, size_t buflen, TIMEVAL stamp, char sep);
void Omni_DumpHeader(FILE *strm, const char *hostname);
void Omni_DumpOMNIHPPOS(FILE *strm, HPPosStruct *hppos);
int  Omni_ReadOMNIHPPOSFromFile(FILE *strm, HPPosStruct *hppos);
unsigned int Omni_ConvToOmniBaud(int Bbaud);
void Omni_PrintHPPosStruct(HPPosStruct *hppos);
void Omni_ClearHPPos(HPPosStruct *hppos);
int  Omni_SetSeed(SeedSetupStruct seed);
int  Omni_RestoreSeed();
int  Omni_StoreSeed();
int  Omni_ResetSeed();
int Omni_IsMsgOfType(OMNI_MSG_TYPE msg_type, const char *buff);
OMNI_MSG_TYPE Omni_GetMsgType(const char *buff);
TIMEVAL Omni_GetMeasuredTime(TIMEVAL itow_change_time, TIMEVAL gps_avail_time);

#endif
