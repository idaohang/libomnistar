/***************************************************************************
                        omnistar.h  -  description
                            -------------------
  begin                : April 2005
  copyright            : (C) 2005 by Pierre Lamon
  email                : pierre.lamon@epfl.ch

  description          : private structures of the library
 ***************************************************************************/

#ifndef OMNISTAR_H
#define OMNISTAR_H

#include "libomnistar_struct.h"

#define OMNI_MIN_FREQ         1525000
#define OMNI_MAX_FREQ         1560000
#define OMNI_MAX_CMD_SIZE     1024
#define OMNI_MSG_START_CHAR   '#'
#define OMNI_REPLY_BEGIN_CHAR '<'
#define OMNI_REPLY_END_CHAR   ']'
#define OMNI_MAX_FIELD_SIZE   32
#define REPLY_OK_SIZE         5
#define REPLY_ERROR_SIZE      27
#define CR                    0x0D
#define LF                    0x0A

#define OMNI_CHECK_FOUND(fun) if(!(fun)) return 0

typedef struct _ComStruct {
  int ttyCom; /* file descriptor */
  int baud;   /* baud rate */
}ComStruct;

void PrintRawBuffer(unsigned char *buf, unsigned int size);
unsigned int ConvToBps(OMNI_BAUD_TYPE baud_type);
unsigned int Omni_ConvToStdBaud(int omni);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char *ucBuffer);
void PrepareHPPOSDumpFormat(char *fmt);
void PrepareHPPOSReadFormat(char *fmt);

#endif

