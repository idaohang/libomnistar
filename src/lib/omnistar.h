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

#include <libelrob/gps.h>

#define OMNI_MAX_BUFF_SIZE 2048

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

typedef enum _OMNI_BAUD_TYPE {OMNI_BAUD_300,
              OMNI_BAUD_600,
              OMNI_BAUD_1200,
              OMNI_BAUD_2400,
              OMNI_BAUD_4800,
              OMNI_BAUD_9600,
              OMNI_BAUD_19200,
              OMNI_BAUD_38400,
              OMNI_BAUD_57600,
              OMNI_BAUD_115200,
              OMNI_BAUD_230400} OMNI_BAUD_TYPE;

typedef enum _OMNI_COM_TYPE {OMNI_COM1,OMNI_COM2,OMNI_COM3} OMNI_COM_TYPE;

typedef enum _OMNI_SETUP_PORT_TYPE{OMNI_SETUP_PORT_COM1,
              OMNI_SETUP_PORT_COM2,
              OMNI_SETUP_PORT_COM3,
              OMNI_SETUP_PORT_THISPORT,
              OMNI_SETUP_PORT_ALL} OMNI_SETUP_PORT_TYPE;

typedef enum _OMNI_PARITY_TYPE {OMNI_PARITY_NO, OMNI_PARITY_EVEN,
OMNI_PARITY_ODD} OMNI_PARITY_TYPE;
typedef enum _OMNI_HANDSHAKE_TYPE {OMNI_HANDSHAKE_NO, OMNI_HANDSHAKE_XON,
OMNI_HANDSHAKE_CTS} OMNI_HANDSHAKE_TYPE;
typedef enum _OMNI_DATABITS_TYPE {OMNI_DATABITS_7 = 7, OMNI_DATABITS_8 = 8}
OMNI_DATABITS_TYPE;
typedef enum _OMNI_STOPBITS_TYPE {OMNI_STOPBITS_1 = 1, OMNI_STOPBITS_2 = 2}
OMNI_STOPBITS_TYPE;
typedef enum _OMNI_ONOFF_TYPE {OMNI_OFF,OMNI_ON,OMNI_ONOFF_END} OMNI_ONOFF_TYPE;

typedef enum _OMNI_LOG_NMEA_TYPE
{OMNI_LOG_ALM,OMNI_LOG_GGA,OMNI_LOG_GLL,OMNI_LOG_GRS,OMNI_LOG_GSA,OMNI_LOG_GST,
OMNI_LOG_GSV,OMNI_LOG_RMC,OMNI_LOG_VTG,OMNI_LOG_ZDA} OMNI_LOG_NMEA_TYPE;

typedef enum _OMNI_MSG_TYPE {OMNI_ALM,OMNI_GGA,
              OMNI_GLL,OMNI_GRS,
              OMNI_GSA,OMNI_GST,
              OMNI_GSV,OMNI_RMC,
              OMNI_VTG,OMNI_ZDA,
              OMNI_GPGGARTK,
              OMNI_HPPOS,
              OMNI_INFO,
              OMNI_STAT,
              OMNI_BESTXYZ,
              OMNI_MSG_END} OMNI_MSG_TYPE;

typedef enum _OMNI_TRIGGER_TYPE {OMNI_TRIGGER_ONNEW,
              OMNI_TRIGGER_ONCHANGED,
              OMNI_TRIGGER_ONTIME,
              OMNI_TRIGGER_ONNEXT,
              OMNI_TRIGGER_ONCE,
              OMNI_TRIGGER_ONMARK,
              OMNI_TRIGGER_END} OMNI_TRIGGER_TYPE;


typedef enum _OMNI_HOLD_TYPE {OMNI_NOHOLD,OMNI_HOLD,OMNI_HOLD_END}
OMNI_HOLD_TYPE;

typedef enum _OMNI_SEED_MODE_TYPE {OMNI_SEED_MODE_RESET, OMNI_SEED_MODE_SET,
OMNI_SEED_MODE_STORE, OMNI_SEED_MODE_RESTORE, OMNI_SEED_MODE_END}
OMNI_SEED_MODE_TYPE;
typedef enum _OMNI_DATUM_TYPE {OMNI_DATUM_WGS84,OMNI_DATUM_TOTO,OMNI_DATUM_END}
OMNI_DATUM_TYPE;
typedef enum _OMNI_UNDULATION_TYPE
{OMNI_UNDULATION_TABLE,OMNI_UNDULATION_USER,OMNI_UNDULATION_OSU89B,
OMNI_UNDULATION_EGM96,OMNI_UNDULATION_END} OMNI_UNDULATION_TYPE;
typedef enum _OMNI_SWITCH_TYPE
{OMNI_SWITCH_DISABLE,OMNI_SWITCH_ENABLE,OMNI_SWITCH_END} OMNI_SWITCH_TYPE;
typedef enum _OMNI_SOLSTATUS_TYPE {OMNI_SOL_COMPUTED,
              OMNI_INSUFFICIENT_OBS,
              OMNI_NO_CONVERGENCE,
              OMNI_SINGULARITY,
              OMNI_COV_TRACE,
              OMNI_TEST_DIST,
              OMNI_COLD_START,
              OMNI_V_H_LIMIT,
              OMNI_VARIANCE,
              OMNI_RESIDUALS,
              OMNI_DELTA_POS,
              OMNI_NEGATIVE_VAR,
              OMNI_RESERVED_S1,
              OMNI_INTEGRITY_WARNING,
              OMNI_SOLSTATUS_END} OMNI_SOLSTATUS_TYPE;

typedef enum _OMNI_POSITION_TYPE {OMNI_NONE,
              OMNI_FIXEDPOS,
              OMNI_FIXEDHEIGHT,
              OMNI_RESERVED_P1,
              OMNI_RESERVED_P2,
              OMNI_RESERVED_P3,
              OMNI_RESERVED_P4,
              OMNI_RESERVED_P5,
              OMNI_DOPPLER_VELOCITY,
              OMNI_RESERVED_P6,
              OMNI_RESERVED_P7,
              OMNI_RESERVED_P8,
              OMNI_RESERVED_P9,
              OMNI_RESERVED_P10,
              OMNI_RESERVED_P11,
              OMNI_RESERVED_P12,
              OMNI_SINGLE,
              OMNI_PSRDIFF,
              OMNI_WAAS,
              OMNI_PROPAGATED,
              OMNI_OMNISTAR,
              OMNI_RESERVED_P13,
              OMNI_RESERVED_P14,
              OMNI_RESERVED_P15,
              OMNI_RESERVED_P16,
              OMNI_RESERVED_P17,
              OMNI_RESERVED_P18,
              OMNI_RESERVED_P19,
              OMNI_RESERVED_P20,
              OMNI_RESERVED_P21,
              OMNI_RESERVED_P22,
              OMNI_RESERVED_P23,
              OMNI_L1_FLOAT,
              OMNI_IONOFREE_FLOAT,
              OMNI_NARROW_FLOAT,
              OMNI_L1_INT,
              OMNI_WIDE_INT,
              OMNI_NARROW_INT,
              OMNI_RTK_DIRECT_INS,
              OMNI_INS1,
              OMNI_INS2,
              OMNI_INS3,
              OMNI_INS4,
              OMNI_INS5,
              OMNI_OMNISTAR_HP,
              OMNI_OMNISTAR_XP,
              OMNI_CDGPS,
              OMNI_POSITION_END} OMNI_POSITION_TYPE;


typedef struct _ComSetupStruct
{
  OMNI_SETUP_PORT_TYPE  port;
  OMNI_BAUD_TYPE        baud;
  OMNI_PARITY_TYPE      parity;
  OMNI_DATABITS_TYPE    databits;
  OMNI_STOPBITS_TYPE    stopbits;
  OMNI_HANDSHAKE_TYPE   handshake;
  OMNI_ONOFF_TYPE       echo;
  OMNI_ONOFF_TYPE       break_;
} ComSetupStruct;

typedef struct _LogSetupStruct
{
  OMNI_SETUP_PORT_TYPE  port;
  OMNI_MSG_TYPE         message;
  OMNI_TRIGGER_TYPE     trigger;
  double                period;
  double                offset;
  OMNI_HOLD_TYPE        hold;
} LogSetupStruct;

typedef struct _SeedSetupStruct
{
  OMNI_SEED_MODE_TYPE   mode;
  GPS_Struct            coord;
  GPS_Struct            sigma;
  OMNI_DATUM_TYPE       datum;
  OMNI_UNDULATION_TYPE  undulation;
} SeedSetupStruct;

/* Maps the OMNIHPPOS message from the 8300 (ignore the header, search for the
first occurence of ;) */
typedef struct _HPPosStruct
{
  TIMEVAL               meas_time;
  TIMEVAL               avail_time;
  int                   valid;
  unsigned long         meas_id;
  /* km, what is this solstatus_type!?! */
  /* km, ASSUMPTION -> this sol_status
     actually means that the data are computed,
     since in the localization.c function, the
     OMNI_SOL_COMPUTED must be the status here,
     otherwise the ominstar message is rejected later on */
  OMNI_SOLSTATUS_TYPE   sol_status;
  /* km, IMPORTANT: the position type of the gps data */
  OMNI_POSITION_TYPE    pos_type;
  /* km, this is the real data from the sensor
     in th gps coordinates */
  GPS_Struct            coord;
  double                undulation;
  OMNI_DATUM_TYPE       datum;
  /* km, the sensor already gives an estimated
     variance of the data .... interesting */
  EPOINT3D              sigma;           /* In meters */
  /* km, station identification (why only one ?!?) */
  int                   station_id;
  /* km, some data i don't really know what they are */
  double                diff_age;
  double                sol_age;
  int                   obs;
  int                   GPSL1;
  int                   L1;
  int                   L2;
} HPPosStruct;

typedef struct _ComStruct {
  int ttyCom; /* file descriptor */
  int baud;   /* baud rate */
}ComStruct;

void PrintRawBuffer(unsigned char *buf, unsigned int size);
unsigned int ConvToBps(OMNI_BAUD_TYPE baud_type);
unsigned int Omni_ConvToStdBaud(int omni);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char
*ucBuffer);
void PrepareHPPOSDumpFormat(char *fmt);
void PrepareHPPOSReadFormat(char *fmt);

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
