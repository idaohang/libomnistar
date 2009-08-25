/***************************************************************************
                        libomnistar.c  -  description
                            -------------------
  begin                : April 2005
  copyright            : (C) 2005 by Pierre Lamon
  email                : pierre.lamon@epfl.ch

  description          : commands for configuring the omnistar. The unit respond with CR LF string CR LF [COMx]
 ***************************************************************************/
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <elrob/serial_easy.h>
#include <elrob/libgps.h>
#include <elrob/gps-nmea.h>
#include "omnistar.h"
#include <elrob/Etime.h>
#include <elrob/Edebug.h>
#include <elrob/Emacros.h>

/* Constants */
const char CMD_ASSIG[]    = "ASSIGNOMNI";
const char CMD_COM[]      = "COM";
const char CMD_LOG[]      = "LOG";
const char CMD_UNLOG[]    = "UNLOG";
const char CMD_UNLOGALL[] = "UNLOGALL";
const char CMD_PSR[]      = "PSRDIFFSOURCE";
const char CMD_RTK[]      = "RTKSOURCE";
const char CMD_RESET[]    = "RESET";
const char CMD_HPSEED[]   = "HPSEED";
const char CMD_SAVECONFIG[] = "SAVECONFIG";
const char ARG_OMNISTAR[] = "OMNISTAR";
const char REPLY_ERROR[]  = "<ER";
const char REPLY_OK[]     = "<OK";
const char SETUP_PORT_STRING[OMNI_SETUP_PORT_ALL+1][10] = {"COM1","COM2","COM3","THISPORT","ALL"};
const char PARITY_STRING[OMNI_PARITY_ODD+1]             = {'N','E','O'};
const char HANDSHAKE_STRING[OMNI_HANDSHAKE_CTS+1][4]    = {"N","XON","CTS"};
const char ONOFF_STRING[OMNI_ON+1][4]  = {"OFF","ON"};
const char MSG_STRING[OMNI_MSG_END][32] = {"GPALM","GPGGA","GPGLL","GPGRS","GPGSA","GPGST","GPGSV","GPRMC","GPVTG","GPZDA","GPGGARTK","OMNIHPPOSA","OMNIINFOA","OMNISTATA","OMNIBESTXYZA"};
size_t SIZE_STRING[OMNI_MSG_END] = {5,5,5,5,5,5,5,5,5,5,8,9,8,8,11};
const char TRIGGER_STRING[OMNI_TRIGGER_END][OMNI_MAX_FIELD_SIZE]      = {"ONNEW","ONCHANGED","ONTIME","ONNEXT","ONCE","ONMARK"};
const char HOLD_STRING[OMNI_HOLD_END][OMNI_MAX_FIELD_SIZE]                    = {"NOHOLD","HOLD"};
const char SEED_MODE_STRING[OMNI_SEED_MODE_END][OMNI_MAX_FIELD_SIZE]  = {"RESET", "SET", "STORE", "RESTORE"};
const char DATUM_STRING[OMNI_DATUM_END][OMNI_MAX_FIELD_SIZE]            = {"WGS84","TOTO"};
const char UNDULATION_STRING[OMNI_UNDULATION_END][OMNI_MAX_FIELD_SIZE] = {"TABLE","USER","OSU89B","EGM96"};
const char SWITCH_STRING[OMNI_SWITCH_END][OMNI_MAX_FIELD_SIZE]        = {"DISABLE","ENABLE"};
const char SOLSTATUS_STRING[OMNI_SOLSTATUS_END][OMNI_MAX_FIELD_SIZE] = {"SOL_COMPUTED","INSUFFICIENT_OBS","NO_CONVERGENCE","SINGULARITY","COV_TRACE","TEST_DIST","COLD_START","V_H_LIMIT","VARIANCE","RESIDUALS","DELTA_POS","NEGATIVE_VAR","INTEGRITY_WARNING"};
    const char POSITION_STRING[OMNI_POSITION_END][OMNI_MAX_FIELD_SIZE] = {"NONE","FIXEDPOS","FIXEDHEIGHT","","","","","","DOPPLER_VELOCITY","","","","","","","","SINGLE","PSRDIFF","WAAS","PROPAGATED","OMNISTAR","","","","","","","","","","","","L1_FLOAT","IONOFREE_FLOAT","NARROW_FLOAT","L1_INT","WIDE_INT","NARROW_INT","RTK_DIRECT_INS","INS1","INS2","INS3","INS4","INS5","OMNISTAR_HP","OMNISTAR_XP","CDGPS"};

static char OMNI_FMT_READ_OMNIHPPOS[256] = "#";
static char OMNI_FMT_DUMP_OMNIHPPOS[256] = "#";

/* The unit return this
<ERROR:Invalid Message ID
in case of error and this
<OK
[COMX]
in case of success (X is the number of the port) */

/* Global variables */
static ComStruct ttyCom[3] = {{-1,-1},{-1,-1},{-1,-1}};   /* contain the file descriptors for the ports*/
static OMNI_COM_TYPE cmd_port = OMNI_COM1;


int Omni_OpenPort(OMNI_COM_TYPE port, char *device, OMNI_BAUD_TYPE baudrate, int timeout)
{
  if(port > OMNI_COM3) {
    fprintf(stderr, "Omni_OpenPort -> Wrong port number (%i)!\n", port);
    return 0;
  }

  ttyCom[port].ttyCom = OpenSerial(device, Omni_ConvToStdBaud(baudrate), timeout,0);
  ttyCom[port].baud   = baudrate;

  return ttyCom[port].ttyCom;
}

int Omni_ClosePort(OMNI_COM_TYPE port)
{
  if(port > OMNI_COM3) {
    fprintf(stderr, "Omni_ClosePort -> Wrong port number (%i)!\n", port);
    return 0;
  }

  CloseSerial(ttyCom[port].ttyCom);
  ttyCom[port].ttyCom = -1;
  ttyCom[port].baud   = -1;
  return 1;
}

int  Omni_SetDefaultCmdPort(OMNI_COM_TYPE def_port)
{
  if(def_port < 0 || def_port > OMNI_COM3) return 0;

  cmd_port = def_port;
  return 1;
}

void Omni_InitDefaultComSetup(ComSetupStruct *com_str)
{
  com_str->port       = OMNI_SETUP_PORT_THISPORT;
  com_str->baud       = OMNI_BAUD_9600;
  com_str->parity     = OMNI_PARITY_NO;
  com_str->databits   = OMNI_DATABITS_8;
  com_str->stopbits   = OMNI_STOPBITS_1;
  com_str->handshake  = OMNI_HANDSHAKE_NO;
  com_str->echo       = OMNI_OFF;
  com_str->break_     = OMNI_ON;
}

/* if OK then return 1 else return 0*/
int CheckReply(char *reply)
{
  if(strncmp(reply,REPLY_OK,3) == 0)
    return 1;
  else
    return 0;
}

int WaitReply(OMNI_COM_TYPE port, char *rec_buf, size_t *rec_bytes)
{
  int  idx = 0;
  int  res;
  int  start_reply = 0;
  int  retok = 0;

  *rec_bytes = 0;

  do
  {
    res = SERIAL_READ(ttyCom[port].ttyCom,&rec_buf[idx],1);

    if(res != 1) {
      EDBG("WaitReply -> could not read 1 byte on port %i\n", port);
      return 0;
    }

    /* Detect the start of the reply */
    if(rec_buf[idx] == OMNI_REPLY_BEGIN_CHAR) {
      idx = 0;
      start_reply = 1;
    }

    /* Check if OK or ERROR */
    if(idx == 2) {
      if(CheckReply(rec_buf))
        retok = 1;
      else
        retok = 0;
    }

    /* Detect the end of the reply (in case it is OK and ERROR) */
    if((rec_buf[idx] == OMNI_REPLY_END_CHAR && start_reply) || (rec_buf[idx] == LF && start_reply && !retok))
    {
      idx += 1;
      rec_buf[idx] = '\0';
      *rec_bytes = idx;
      return retok;
    }

    if(start_reply)
      idx++;

  }
  while(idx < OMNI_MAX_CMD_SIZE);

  return 0;
}

/* return 0 if failure else 1. if check != 0 then read the reply */
int SendCmd(char *cmd, OMNI_COM_TYPE port, int check)
{
  int          size;
  int          res;
  size_t       rec_bytes;
  char         rec_buf[OMNI_MAX_CMD_SIZE];

  /* Write the command (append CR,LF) */
  EDBG("SendCmd -> Command sent on port %i \"%s (+CR+LF)\"", port, cmd);
  size = strlen(cmd);
  cmd[size]   = CR;
  cmd[size+1] = LF;
  cmd[size+2] = '\0';

  SERIAL_FLUSH_RX(ttyCom[port].ttyCom);

  size += 2;
  res  = SERIAL_WRITE(ttyCom[port].ttyCom,cmd,size);
  if(res != size) {
    EDBG("SendCmd -> could not write %i bytes on port %i\n", size, port);
    return 0;
  }

  /* Read the reply */
  if (check)
    res = WaitReply(port, rec_buf, &rec_bytes);
  else res = 1;

  EDBG("SendCmd -> Received \"%s\"", rec_buf);

  /* Check if ok or not */
  return res;
}


int Omni_AssignOmni(unsigned int freq, OMNI_BAUD_TYPE baud)
{
  char cmd[OMNI_MAX_CMD_SIZE];

  if(freq < OMNI_MIN_FREQ || freq > OMNI_MAX_FREQ) return 0;

  sprintf(cmd,"%s USER %i %i", CMD_ASSIG, freq, ConvToBps(baud));

  return SendCmd(cmd, cmd_port, 1);
}

int Omni_ComSetup(ComSetupStruct com_str)
{
  char cmd[OMNI_MAX_CMD_SIZE];
  int  res;

  sprintf(cmd,"%s %s,%i,%c,%i,%i,%s,%s,%s",
              CMD_COM,
              SETUP_PORT_STRING[com_str.port],
              ConvToBps(com_str.baud),
              PARITY_STRING[com_str.parity],
              com_str.databits,
              com_str.stopbits,
              HANDSHAKE_STRING[com_str.handshake],
              ONOFF_STRING[com_str.echo],
              ONOFF_STRING[com_str.break_]);

  res = SendCmd(cmd, cmd_port, 1);
  return res;
}

int  Omni_LogSetup(LogSetupStruct log_str)
{
  char cmd[OMNI_MAX_CMD_SIZE];

  sprintf(cmd,"%s %s %s %s %.3f %.3f %s",
          CMD_LOG,
          SETUP_PORT_STRING[log_str.port],
          MSG_STRING[log_str.message],
          TRIGGER_STRING[log_str.trigger],
          log_str.period,
          log_str.offset,
          HOLD_STRING[log_str.hold]);

  return SendCmd(cmd, cmd_port, 1);
}

int  Omni_UnlogMsg(OMNI_SETUP_PORT_TYPE port, OMNI_MSG_TYPE msg)
{
  char cmd[OMNI_MAX_CMD_SIZE];

  sprintf(cmd,"%s %s %s",
          CMD_UNLOG,
          SETUP_PORT_STRING[port],
          MSG_STRING[msg]);

  return SendCmd(cmd, cmd_port, 1);
}

int  Omni_UnlogAll(int check_reply)
{
  char cmd[OMNI_MAX_CMD_SIZE];
  sprintf(cmd,"%s", CMD_UNLOGALL);
  return SendCmd(cmd, cmd_port, check_reply);
}

int  Omni_EnableHP()
{
  char cmd[OMNI_MAX_CMD_SIZE];
  sprintf(cmd,"%s %s", CMD_RTK, ARG_OMNISTAR);
  return SendCmd(cmd, cmd_port, 1);
}

int  Omni_EnableVBS()
{
  char cmd[OMNI_MAX_CMD_SIZE];
  sprintf(cmd,"%s %s", CMD_PSR, ARG_OMNISTAR);
  return SendCmd(cmd, cmd_port, 1);
}

int  Omni_SaveConfig()
{
  char cmd[OMNI_MAX_CMD_SIZE];
  sprintf(cmd,"%s", CMD_SAVECONFIG);
  return SendCmd(cmd, cmd_port, 1);
}

int  Omni_Reset(unsigned int seconds)
{
  char cmd[OMNI_MAX_CMD_SIZE];
  sprintf(cmd,"%s %i", CMD_RESET, seconds);
  return SendCmd(cmd, cmd_port, 1);
}

int Omni_SetSeed(SeedSetupStruct seed)
{
  char cmd[OMNI_MAX_CMD_SIZE];

  sprintf(cmd,"%s %s %.11f %.11f %.4f %.4f %.4f %.4f %s %s",
          CMD_HPSEED,
          SEED_MODE_STRING[seed.mode],
          RAD2DEG(seed.coord.latitude),
          RAD2DEG(seed.coord.longitude),
          seed.coord.altitude,
          RAD2DEG(seed.sigma.latitude),
          RAD2DEG(seed.sigma.longitude),
          seed.sigma.altitude,
          DATUM_STRING[seed.datum],
          UNDULATION_STRING[seed.undulation]);

  return SendCmd(cmd, cmd_port, 1);
}

int Omni_StoreSeed()
{
  char cmd[OMNI_MAX_CMD_SIZE];
  sprintf(cmd,"%s STORE", CMD_HPSEED);
  return SendCmd(cmd, cmd_port, 1);
}

int Omni_RestoreSeed()
{
  char cmd[OMNI_MAX_CMD_SIZE];
  sprintf(cmd,"%s RESTORE", CMD_HPSEED);
  return SendCmd(cmd, cmd_port, 1);
}

int  Omni_ResetSeed()
{
  char cmd[OMNI_MAX_CMD_SIZE];
  sprintf(cmd,"%s RESET", CMD_HPSEED);
  return SendCmd(cmd, cmd_port, 1);
}

/* return 0 if not valid */
int Omni_ParseStream(OMNI_COM_TYPE port, char *rec_buf, size_t buflen, TIMEVAL *stamp)
{
  int   sync = 0;
  char  *ptr;
  int   valid = 0;
  int   OMNI_msg = 0;
  int   res;

  rec_buf[0] = '\0';
  ptr = rec_buf;

  while(1)
  {
    /* Read bytes one by one */
    res = SERIAL_READ(ttyCom[port].ttyCom,ptr,1);
    if(res != 1) {
      /*EERR("Could not read one byte\n");*/
      rec_buf[0] = '\0';
      return 0;
    }

    /* detects the start of a message */
    if(*ptr == GPS_NMEA_START_CHAR || *ptr == OMNI_MSG_START_CHAR || *ptr == OMNI_REPLY_BEGIN_CHAR)
    {
      /* distinguish between $ messages and unit replies */
      if(*ptr == OMNI_REPLY_BEGIN_CHAR) {
        EERR("You should never end in this stuff. This is old mode of communication\n");
        OMNI_msg += 1;
        valid = 0;
      }
      else{
        OMNI_msg = 0;
        valid = 1;
      }

      /* rewind ptr */
      if(OMNI_msg <= 1)
      {
        rec_buf[0] = *ptr; /* put the start character at the beginning of the buffer */
        ptr = (char *) (rec_buf+1);
      }
      else {
        ptr++;
      }

      /* we are in sync now -> stack what follows in rec_buf */
      sync = 1;
    }
    else if(*ptr == LF)   /* Detect the end of the message */
    {
      if(sync && OMNI_msg == 0)
      {
        Time_gettimeofday(stamp);  /* stamp here */
        ptr++;
        *ptr = '\0';
        return valid;
      }
      else if (OMNI_msg == 1)
      {
        ptr++;
      }
      else if(OMNI_msg == 2)
      {
        Time_gettimeofday(stamp);  /* stamp here */
        ptr++;
        *ptr = '\0';
        return 1;
      }
      else
        sync = 0; /* from now wait until we resync is received */
    }
    else
        ptr++;

    if(ptr > (char *) (rec_buf + buflen))
    {
      EERR("Omni_ParseStream -> Buffer overflow!\n");
      rec_buf[0] = '\0';
      return 0;
    }
  }

  return 0;
}

void Omni_DebugPrintField(const char *field, size_t size)
{
  char str[128];

  strncpy(str,field,size);
  str[size] = '\0';
  EDBG("Field: %s",str);
}

/* Return the enum in case of a match is found else -1 */
int Omni_FindMatch(const char STRING[][OMNI_MAX_FIELD_SIZE], size_t size, const char *field, size_t length)
{
  int i;

  for(i = 0; i < size; i++){
    /*EPRINT("{%s}, ",STRING[i]);*/
    if(strncmp(field,STRING[i],length) == 0) {
      EDBG("found (%i)\n",i);
      return i;
    }
  }

  return -1;
}

OMNI_SOLSTATUS_TYPE Omni_GetSolStatus(const char *field, size_t size)
{
  return Omni_FindMatch(SOLSTATUS_STRING, OMNI_SOLSTATUS_END, field, size);
}

OMNI_SOLSTATUS_TYPE Omni_GetPosType(const char *field, size_t size)
{
  return Omni_FindMatch(POSITION_STRING, OMNI_POSITION_END, field, size);
}

OMNI_DATUM_TYPE Omni_GetDatum(const char *field, size_t size)
{
  return Omni_FindMatch(DATUM_STRING, OMNI_DATUM_END, field, size);
}

inline double Omni_ReadDouble(const char *field, size_t size)
{
  double res;
  char   str[128];

  strncpy(str,field,size);
  str[size] = '\0';

  if(sscanf(str,"%lf",&res) != 1) {
    EERR("Omni_ReadDouble -> Could not read field");
    return 0.0;
  }
  return res;
}

inline int Omni_ReadInt(const char *field, size_t size)
{
  char str[128];

  strncpy(str,field,size);
  str[size] = '\0';
  return atoi(str);
}

/* search the next comma, return 0 in case of error
   ptr_next point on the next field, ptr_field points on the first char of the field */
inline int Omni_GetNextField(char *ptr_field, char **ptr_next, size_t *size, char sep)
{
  *ptr_next = strchr(ptr_field, sep);
  if (*ptr_next == NULL) return 0;

  *size = (size_t) (*ptr_next - ptr_field);
  *ptr_next += 1;

  Omni_DebugPrintField(ptr_field,*size);
  return 1;
}

/* Return 1 if ok else 0 */
int Omni_CheckCRC(const char *buffer)
{
  unsigned long crc_computed, crc_field = 0;
  char          *ptr_crc;
  size_t        size;

  ptr_crc = strchr(buffer, '*');
  if (ptr_crc == NULL) return 0;

  ptr_crc += 1;

  sscanf(ptr_crc,"%lx",&crc_field);
  size = (size_t) (ptr_crc-buffer-2);

  crc_computed = CalculateBlockCRC32(size,(unsigned char *) buffer+1);
  EDBG("crc computed: %x crc field: %x size = %i\n", crc_computed, crc_field, size);

  if(crc_field != crc_computed){
    EERR("Omni_CheckCRC -> Wrong CRC\n");
    return 0;
  }
  return 1;
}

void Omni_PrintHPPosStruct(HPPosStruct *hppos)
{
  EPRINT("** HPPosStruct **\n");
  EPRINT("valid:\t\t %i\n", hppos->valid);
  EPRINT("meas_id:\t %i\n", hppos->meas_id);
  EPRINT("sol_status:\t %s (%i)\n", SOLSTATUS_STRING[hppos->sol_status],hppos->sol_status);
  EPRINT("pos_type:\t %s (%i)\n", POSITION_STRING[hppos->pos_type],hppos->pos_type);
  EPRINT("Coordinates-\n");
  GPS_PrintGPS(&hppos->coord);
  EPRINT("undulation:\t %f\n", hppos->undulation);
  EPRINT("datum:\t\t %s\n", DATUM_STRING[hppos->datum]);
  EPRINT("Sigma Coordinates (in meters)-\n");
  EPRINT("Along x:\t\t %f\n", hppos->sigma.x);
  EPRINT("Along y:\t\t %f\n", hppos->sigma.y);
  EPRINT("Along z:\t\t %f\n", hppos->sigma.z);

  EPRINT("station_id:\t %i\n", hppos->station_id);
  EPRINT("diff_age:\t %f\n", hppos->diff_age);
  EPRINT("sol_age:\t %f\n", hppos->sol_age);
  EPRINT("obs:\t\t %i\n", hppos->obs);
  EPRINT("GPSL1:\t\t %i\n", hppos->GPSL1);
  EPRINT("L1:\t\t %i\n", hppos->L1);
  EPRINT("L2:\t\t %i\n", hppos->L2);
  EPRINT("meas_time:\t %f\n", Time_FromTimeval(hppos->meas_time));
  EPRINT("avail_time:\t %f\n", Time_FromTimeval(hppos->avail_time));
}

void Omni_DumpHeader(FILE *strm, const char *hostname)
{
 if(hostname != NULL)
   DUMP_HOSTNAME(strm,hostname);

 fprintf(strm,"# OMNIHPPOS format is : \n");
 fprintf(strm,"# 1:  data row number \n");
 fprintf(strm,"# 2:  measured time [s]\n");
 fprintf(strm,"# 3:  available time [s]\n");
 fprintf(strm,"# 4:  validity\n");
 fprintf(strm,"# 5:  measurement id\n");
 fprintf(strm,"# 6:  sol_status {enum}\n");
 fprintf(strm,"# 7:  pos_type {enum}\n");
 fprintf(strm,"# 8:  latitude  [rad]\n");
 fprintf(strm,"# 9:  longitude [rad]\n");
 fprintf(strm,"# 10: altitude  [rad]\n");
 fprintf(strm,"# 11: undulation [m]\n");
 fprintf(strm,"# 12: datum {enum}\n");
 fprintf(strm,"# 13: sigma latitude  [rad]\n");
 fprintf(strm,"# 14: sigma longitude [rad]\n");
 fprintf(strm,"# 15: sigma altitude  [rad]\n");
 fprintf(strm,"# 16: station id\n");
 fprintf(strm,"# 17: diff age\n");
 fprintf(strm,"# 18: sol age\n");
 fprintf(strm,"# 19: obs\n");
 fprintf(strm,"# 20: GPSL1\n");
 fprintf(strm,"# 21: L1\n");
 fprintf(strm,"# 22: L2\n");
}

void Omni_DumpOMNIHPPOS(FILE *strm, HPPosStruct *hppos)
{
  PrepareHPPOSDumpFormat(OMNI_FMT_DUMP_OMNIHPPOS);

  fprintf(strm,OMNI_FMT_DUMP_OMNIHPPOS,
          MODULE_NAME_OMNIHPPOS,
          Time_FromTimeval(hppos->meas_time),
          Time_FromTimeval(hppos->avail_time),
          hppos->valid,
          hppos->meas_id,
          hppos->sol_status,
          hppos->pos_type,
          hppos->coord.latitude,
          hppos->coord.longitude,
          hppos->coord.altitude,
          hppos->undulation,
          hppos->datum,
          hppos->sigma.x,
          hppos->sigma.y,
          hppos->sigma.z,
          hppos->station_id,
          hppos->diff_age,
          hppos->sol_age,
          hppos->obs,
          hppos->GPSL1,
          hppos->L1,
          hppos->L2);
}

int Omni_ReadOMNIHPPOSFromFile(FILE *strm, HPPosStruct *hppos)
{
  int    data_id;
  double meas_time;
  double avail_time;
  int    res;

  PrepareHPPOSReadFormat(OMNI_FMT_READ_OMNIHPPOS);

  res = fscanf(strm,OMNI_FMT_READ_OMNIHPPOS,
                    &data_id,
                    &meas_time,
                    &avail_time,
                    &hppos->valid,
                    &hppos->meas_id,
                    &hppos->sol_status,
                    &hppos->pos_type,
                    &hppos->coord.latitude,
                    &hppos->coord.longitude,
                    &hppos->coord.altitude,
                    &hppos->undulation,
                    &hppos->datum,
                    &hppos->sigma.x,
                    &hppos->sigma.y,
                    &hppos->sigma.z,
                    &hppos->station_id,
                    &hppos->diff_age,
                    &hppos->sol_age,
                    &hppos->obs,
                    &hppos->GPSL1,
                    &hppos->L1,
                    &hppos->L2);

  if(res != 22){
    EERR("Omni_ReadOMNIHPPOSFromFile -> Could not read all the field\n");
    hppos->valid = 0;
    return 0;
  }

  hppos->meas_time = Time_FromDbl(meas_time);
  hppos->avail_time = Time_FromDbl(avail_time);

  return 1;
}

int  Omni_ParseOMNIHPPOS(HPPosStruct *hppos, const char *rec_buf, size_t buflen, TIMEVAL stamp, char sep)
{
  char   *ptr_field, *ptr_next;
  size_t size;

  hppos->valid = 0;
  hppos->avail_time = stamp;
  hppos->meas_id++;
  ptr_field = (char *) rec_buf + 1;

  if(sep == ',') {
    ptr_field = strchr(rec_buf, ';');
    if (ptr_field == NULL) return 0;
    ptr_field++;
  }
  else {
    ptr_field = strchr(ptr_field, '<');
    if (ptr_field == NULL) return 0;
    ptr_field++;

    while(*ptr_field == sep) ptr_field++;
  }

  if(!Omni_CheckCRC(rec_buf)) return 0;

  /* sol_status */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->sol_status = Omni_GetSolStatus(ptr_field,size);
  if(hppos->sol_status == -1) return 0;
  ptr_field = ptr_next;

  /* sol_pos */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->pos_type = Omni_GetPosType(ptr_field,size);
  if(hppos->pos_type == -1) return 0;
  ptr_field = ptr_next;

  /* Lat */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->coord.latitude = -DEG2RAD(Omni_ReadDouble(ptr_field,size));
  ptr_field = ptr_next;

  /* Lon */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->coord.longitude = -DEG2RAD(Omni_ReadDouble(ptr_field,size));
  ptr_field = ptr_next;

  /* Hgt */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->coord.altitude = Omni_ReadDouble(ptr_field,size);
  ptr_field = ptr_next;

  /* Undulation */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->undulation = Omni_ReadDouble(ptr_field,size);
  ptr_field = ptr_next;

  /* datum */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->datum = Omni_GetDatum(ptr_field,size);
  if(hppos->datum == -1) return 0;
  ptr_field = ptr_next;

  /* Lat sigma */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->sigma.x = Omni_ReadDouble(ptr_field,size);
  ptr_field = ptr_next;

  /* Lon sigma */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->sigma.y = Omni_ReadDouble(ptr_field,size);
  ptr_field = ptr_next;

  /* Hgt sigma */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->sigma.z = Omni_ReadDouble(ptr_field,size);
  ptr_field = ptr_next;

  /* Station id */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->station_id = Omni_ReadInt(ptr_field,size);
  ptr_field = ptr_next;

  /* Diff age */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->diff_age = Omni_ReadDouble(ptr_field,size);
  ptr_field = ptr_next;

  /* Sol age */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->sol_age = Omni_ReadDouble(ptr_field,size);
  ptr_field = ptr_next;

  /* Obs */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->obs = Omni_ReadInt(ptr_field,size);
  ptr_field = ptr_next;

  /* GPSL1 */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->GPSL1 = Omni_ReadInt(ptr_field,size);
  ptr_field = ptr_next;

  /* L1 */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->L1 = Omni_ReadInt(ptr_field,size);
  ptr_field = ptr_next;

  /* L2 */
  OMNI_CHECK_FOUND(Omni_GetNextField(ptr_field, &ptr_next, &size, sep));
  hppos->L2 = Omni_ReadInt(ptr_field,size);
  ptr_field = ptr_next;

  /*  if(sep == ',') {
    ptr_field = strchr(ptr_field, '*');
    if (ptr_field == NULL) return 0;
    ptr_field += 1;
  }*/

  hppos->valid = 1;

  return 1;
}

unsigned int Omni_ConvToOmniBaud(int Bbaud)
{
  switch(Bbaud)
  {
    case B300:    return OMNI_BAUD_300; break;
    case B600:    return OMNI_BAUD_600; break;
    case B1200:   return OMNI_BAUD_1200;break;
    case B2400:   return OMNI_BAUD_2400;break;
    case B4800:   return OMNI_BAUD_4800;break;
    case B9600:   return OMNI_BAUD_9600;break;
    case B19200:  return OMNI_BAUD_19200;break;
    case B38400:  return OMNI_BAUD_38400;break;
    case B57600:  return OMNI_BAUD_57600;break;
    case B115200: return OMNI_BAUD_115200;break;
    case B230400: return OMNI_BAUD_230400;break;
    default: return B9600;
  }
}

void Omni_ClearHPPos(HPPosStruct *hppos)
{
  bzero(hppos, sizeof(hppos));
}

inline int Omni_IsMsgOfType(OMNI_MSG_TYPE msg_type, const char *buff)
{
  const char   *ptr_buf = buff + 1;
  const char   *ptr_str = MSG_STRING[msg_type];

  if(msg_type >= OMNI_MSG_END)
    return 0;

    if(!strncmp(ptr_str, ptr_buf, SIZE_STRING[msg_type]))
      return 1;
    else
      return 0;
}

/* return -1 in case of error */
OMNI_MSG_TYPE Omni_GetMsgType(const char *buff)
{
  size_t i;

  for(i = 0; i < OMNI_MSG_END; i++){
    if(Omni_IsMsgOfType(i,buff))
      return (OMNI_MSG_TYPE) i;
  }

  return -1;
}

/* return the measurement time for an omnistar message, if the available time is not close enough to itow_change_time
 then gps_avail_time is taken as the measurement time */
TIMEVAL Omni_GetMeasuredTime(TIMEVAL itow_change_time, TIMEVAL gps_avail_time)
{
  double  itow_change_dbl, gps_avail_dbl;

  itow_change_dbl = Time_FromTimeval(itow_change_time);
  gps_avail_dbl   = Time_FromTimeval(gps_avail_time);

  /* Check if the GPS data is close enough to the itow change time */
  if(Time_CheckCloseDbl(itow_change_dbl, gps_avail_dbl, 0.11))
  {
    return itow_change_time;
  }
  else
  {
    return gps_avail_time;
  }
}


