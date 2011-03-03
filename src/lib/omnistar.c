/***************************************************************************
                        omnistar.c  -  description
                            -------------------
  begin                : April 2005
  copyright            : (C) 2005 by Pierre Lamon
  email                : pierre.lamon@epfl.ch

  description          : private functions for the omnistar library
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include <libelrob/Etime.h>

#include "omnistar.h"

const char OMNI_AXIS_READ_FMT[] = "%lf\t%lf\t%lf\t";
const char OMNI_AXIS_DUMP_FMT[] = "%.15lf\t%.15lf\t%.4lf\t";
const char OMNI_REAL_FMT[] = "%lf\t";
const char OMNI_INT_FMT[]  = "%i\t";
const char OMNI_CHAR_FMT[] = "%c\t";

void PrintRawBuffer(unsigned char *buf, unsigned int size)
{
  size_t  i;

  printf("(%i) ", size);
  for(i = 0; i < size-1; i++) printf( "%.2X, ", buf[i]);
  printf("%.2X\n", buf[i]);
}

/* return the integer corresponding to baud_type */
unsigned int ConvToBps(OMNI_BAUD_TYPE baud_type)
{
  switch(baud_type)
  {
    case OMNI_BAUD_300:  return 300; break;
    case OMNI_BAUD_600:  return 600; break;
    case OMNI_BAUD_1200: return 1200;break;
    case OMNI_BAUD_2400: return 2400;break;
    case OMNI_BAUD_4800: return 4800;break;
    case OMNI_BAUD_9600: return 9600;break;
    case OMNI_BAUD_19200: return 19200;break;
    case OMNI_BAUD_38400: return 38400;break;
    case OMNI_BAUD_57600: return 57600;break;
    case OMNI_BAUD_115200: return 115200;break;
    case OMNI_BAUD_230400: return 230400;break;
    default: return 1200;
  }
}

unsigned int Omni_ConvToStdBaud(int omni)
{
  switch(omni)
  {
    case OMNI_BAUD_300 :    return B300; break;
    case OMNI_BAUD_600 :    return B600; break;
    case OMNI_BAUD_1200 :   return B1200;break;
    case OMNI_BAUD_2400 :   return B2400;break;
    case OMNI_BAUD_4800 :   return B4800;break;
    case OMNI_BAUD_9600 :   return B9600;break;
    case OMNI_BAUD_19200 :  return B19200;break;
    case OMNI_BAUD_38400 :  return B38400;break;
    case OMNI_BAUD_57600 :  return B57600;break;
    case OMNI_BAUD_115200 : return B115200;break;
    case OMNI_BAUD_230400 : return B230400;break;
    default: return OMNI_BAUD_9600;
  }
}

#define CRC32_POLYNOMIAL 0xEDB88320L
/* --------------------------------------------------------------------------
Calculate a CRC value to be used by CRC calculation functions.
-------------------------------------------------------------------------- */
unsigned long CRC32Value(int i)
{
  int j;
  unsigned long ulCRC;
  ulCRC = i;
  for ( j = 8 ; j > 0; j-- )
  {
    if ( ulCRC & 1 )
      ulCRC = ( ulCRC >> 1 ) ^ CRC32_POLYNOMIAL;
    else
      ulCRC >>= 1;
  } return ulCRC;
}
/* --------------------------------------------------------------------------
Calculates the CRC-32 of a block of data all at once
-------------------------------------------------------------------------- */
unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
unsigned char *ucBuffer ) /* Data block */
{
  unsigned long ulTemp1;
  unsigned long ulTemp2;
  unsigned long ulCRC = 0;
  while ( ulCount-- != 0 )
  {
    ulTemp1 = ( ulCRC >> 8 ) & 0x00FFFFFFL;
    ulTemp2 = CRC32Value( ((int) ulCRC ^ *ucBuffer++ ) & 0xff );
    ulCRC = ulTemp1 ^ ulTemp2;
  }
  return( ulCRC );
}

void PrepareHPPOSDumpFormat(char *fmt)
{
  if(fmt[0] == '#')
  {
    strcpy(fmt,OMNI_INT_FMT); /* Data id*/
    strcat(fmt,OMNI_REAL_FMT);
    strcat(fmt,OMNI_REAL_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_AXIS_DUMP_FMT);
    strcat(fmt,OMNI_REAL_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_AXIS_DUMP_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_REAL_FMT);
    strcat(fmt,OMNI_REAL_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    /*strcat(fmt,"\n");*/
  }
}

void PrepareHPPOSReadFormat(char *fmt)
{
  if(fmt[0] == '#')
  {
    strcpy(fmt,OMNI_INT_FMT); /* Data id*/
    strcat(fmt,OMNI_REAL_FMT);
    strcat(fmt,OMNI_REAL_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_AXIS_READ_FMT);
    strcat(fmt,OMNI_REAL_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_AXIS_READ_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_REAL_FMT);
    strcat(fmt,OMNI_REAL_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,OMNI_INT_FMT);
    strcat(fmt,"\n");
  }
}
