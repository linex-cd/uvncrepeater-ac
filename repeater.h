/*
 * Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * Program is based on the
 * http://www.imasy.or.jp/~gotoh/ssh/connect.c
 * Written By Shun-ichi GOTO <gotoh@taiyo.co.jp>
 *
 * If the source code for the program is not available from the place
 * from which you received this file, check
 * http://ultravnc.sourceforge.net/
 *
 * Linux port (C) 2005- Jari Korhonen, jarit1.korhonen@dnainternet.net
 */

#ifndef REPEATER_H
#define REPEATER_H

#include "commondefines.h"



#ifndef REPEATER_VERSION
#define REPEATER_VERSION "devel"
#endif

#define RFB_PROTOCOL_VERSION_FORMAT "RFB %03d.%03d\n"
#define RFB_PROTOCOL_MAJOR_VERSION 0
#define RFB_PROTOCOL_MINOR_VERSION 0
#define SIZE_RFBPROTOCOLVERSIONMSG 12

#define RFB_PORT_OFFSET 5900                /* servers 1st display is in this port number */
#define MAX_IDLE_CONNECTION_TIME 600        /* Seconds */
#define MAX_HOST_NAME_LEN 250
#define MAX_PATH 250
#define MAX_HANDSHAKE_LEN 100
#define UNKNOWN_REPINFO_IND 999999          /* Notice: This should always be bigger than maxSessions */

#define LISTEN_BACKLOG  5                   /* Listen() queues 5 connections */

/* connectionFrom defines for acceptConnection(). Used also in connectionRemover() */
#define CONNECTION_FROM_SERVER 0
#define CONNECTION_FROM_VIEWER 1

/* mode1ConnCode is used in Mode1 to "invent" code field in repeaterInfo,
 * when new Mode1 connection from viewer is accepted. This is just decremented
 * for each new Mode 1 connection to ensure unique number for each Mode 1 session
 * Values for this are:
 *    0 = program has just started,
 *   -1....MIN_INVENTED_CONN_CODE: Codes for each session
 */
#define MIN_INVENTED_CONN_CODE -1000000


extern void debug(int msgLevel, const char *fmt, ...);
extern void fatal(const char *fmt, ...);
extern int openConnectionToEventListener(const char *host, unsigned short port, char *listenerIp, int listenerIpSize);
extern int writeExact(int sock, char *buf, int len, int timeOutSecs);

typedef struct _repeaterInfo {
    int socket;

    /* Code is used for cross-connection between servers and viewers
     * In Mode 2, Server/Viewer sends IdCode string "ID:xxxxx", where xxxxx is
     *   some positive (1 or bigger) long integer number
     * In Mode 1, Repeater "invents" a non-used code (negative number) and
     *   assigns that to both Server/Viewer
     *   code == 0 means that entry in servers[] / viewers[] table is free
     */
    long code;

    unsigned long timeStamp;

    /* Ip address of peer */
    struct in46_addr peerIp;

    /* There are 3 connection levels (using variables "code" and "active"):
     * A. code==0,active==false: fully idle, no connection attempt detected
     * B. code==non-zero,active==false: server/viewer has connected, waiting for other end to connect
     * C. code==non-zero,active=true: doRepeater() running on viewer/server connection, fully active
     * -after viewer/server disconnects or some error in doRepeater, returns both to level A
     * (and closes respective sockets)
     * This logic means, that when one end disconnects, BOTH ends need to reconnect.
     * This is not a bug, it is a feature ;-)
     */
    bool active;
} repeaterInfo;



/* Server handshake strings for use when respective viewer connects later */
typedef struct _handShakeInfo
{
    char handShake[MAX_HANDSHAKE_LEN];
    int handShakeLength;
} handShakeInfo;


/* This structure (and repeaterProcs[] table) is used for
 * keeping track of child processes running doRepeater
 * and cleaning up after they exit
 */
typedef struct _repeaterProcInfo
{
    long code;
    pid_t pid;
} repeaterProcInfo;


#endif
