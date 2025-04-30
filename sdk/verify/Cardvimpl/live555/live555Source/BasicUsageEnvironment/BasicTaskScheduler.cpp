/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2016 Live Networks, Inc.  All rights reserved.
// Basic Usage Environment: for a simple, non-scripted, console application
// Implementation


#include "BasicUsageEnvironment.hh"
#include "HandlerSet.hh"
#include <stdio.h>
#if defined(_QNX4)
#include <sys/select.h>
#include <unix.h>
#endif
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>
#include "GroupsockHelper.hh"

enum {
	TCP_ESTABLISHED = 1,
	TCP_SYN_SENT,
	TCP_SYN_RECV,
	TCP_FIN_WAIT1,
	TCP_FIN_WAIT2,
	TCP_TIME_WAIT,
	TCP_CLOSE,
	TCP_CLOSE_WAIT,
	TCP_LAST_ACK,
	TCP_LISTEN,
	TCP_CLOSING,	/* Now a valid state */
	TCP_NEW_SYN_RECV,

	TCP_MAX_STATES	/* Leave at the end! */
};
int gbackupErrSocket = 0xffff;
////////// BasicTaskScheduler //////////

BasicTaskScheduler* BasicTaskScheduler::createNew(unsigned maxSchedulerGranularity) {
	return new BasicTaskScheduler(maxSchedulerGranularity);
}

BasicTaskScheduler::BasicTaskScheduler(unsigned maxSchedulerGranularity)
  : fMaxSchedulerGranularity(maxSchedulerGranularity), fMaxNumSockets(0)
#if defined(__WIN32__) || defined(_WIN32)
  , fDummySocketNum(-1)
#endif
{
  FD_ZERO(&fReadSet);
  FD_ZERO(&fWriteSet);
  FD_ZERO(&fExceptionSet);

  if (maxSchedulerGranularity > 0) schedulerTickTask(); // ensures that we handle events frequently
}

BasicTaskScheduler::~BasicTaskScheduler() {
#if defined(__WIN32__) || defined(_WIN32)
  if (fDummySocketNum >= 0) closeSocket(fDummySocketNum);
#endif
}

void BasicTaskScheduler::schedulerTickTask(void* clientData) {
  ((BasicTaskScheduler*)clientData)->schedulerTickTask();
}

void BasicTaskScheduler::schedulerTickTask() {
  scheduleDelayedTask(fMaxSchedulerGranularity, schedulerTickTask, this);
}

#ifndef MILLION
#define MILLION 1000000
#endif

void BasicTaskScheduler::SingleStep(unsigned maxDelayTime) {
  fd_set readSet = fReadSet; // make a copy for this select() call
  fd_set writeSet = fWriteSet; // ditto
  fd_set exceptionSet = fExceptionSet; // ditto
  int MaxNumSockets = fMaxNumSockets;
  int numSockets;
  DelayInterval const& timeToDelay = fDelayQueue.timeToNextAlarm();
  struct timeval tv_timeToDelay;
  tv_timeToDelay.tv_sec = timeToDelay.seconds();
  tv_timeToDelay.tv_usec = timeToDelay.useconds();
  // Very large "tv_sec" values cause select() to fail.
  // Don't make it any larger than 1 million seconds (11.5 days)
  const long MAX_TV_SEC = MILLION;
  if (tv_timeToDelay.tv_sec > MAX_TV_SEC) {
    tv_timeToDelay.tv_sec = MAX_TV_SEC;
  }
  // Also check our "maxDelayTime" parameter (if it's > 0):
  if (maxDelayTime > 0 &&
      (tv_timeToDelay.tv_sec > (long)maxDelayTime/MILLION ||
       (tv_timeToDelay.tv_sec == (long)maxDelayTime/MILLION &&
	tv_timeToDelay.tv_usec > (long)maxDelayTime%MILLION))) {
    tv_timeToDelay.tv_sec = maxDelayTime/MILLION;
    tv_timeToDelay.tv_usec = maxDelayTime%MILLION;
  }

  int selectResult = select(fMaxNumSockets, &readSet, &writeSet, &exceptionSet, &tv_timeToDelay);
  if (selectResult < 0) {
#if defined(__WIN32__) || defined(_WIN32)
    int err = WSAGetLastError();
    // For some unknown reason, select() in Windoze sometimes fails with WSAEINVAL if
    // it was called with no entries set in "readSet".  If this happens, ignore it:
    if (err == WSAEINVAL && readSet.fd_count == 0) {
      err = EINTR;
      // To stop this from happening again, create a dummy socket:
      if (fDummySocketNum >= 0) closeSocket(fDummySocketNum);
      fDummySocketNum = socket(AF_INET, SOCK_DGRAM, 0);
      FD_SET((unsigned)fDummySocketNum, &fReadSet);
    }
    if (err != EINTR) {
#else
    if (errno != EINTR && errno != EAGAIN) {
#endif
	// Unexpected error - treat this as fatal:
#if !defined(_WIN32_WCE)
    	perror("BasicTaskScheduler::SingleStep(): select() fails");

    	// Because this failure is often "Bad file descriptor" - which is caused by an invalid socket number (i.e., a socket number
    	// that had already been closed) being used in "select()" - we print out the sockets that were being used in "select()",
    	// to assist in debugging:
        #define MAX(x,y) ((x) > (y) ? (x) : (y))
        #define MIN(x,y) ((x) > (y) ? (y) : (x))
        numSockets = MAX(MaxNumSockets, fMaxNumSockets);
        numSockets = MIN(numSockets, 10000);
    	fprintf(stderr, "socket numbers used in the select(%d) call:[%d,%d,%d]",errno,MaxNumSockets,fMaxNumSockets,numSockets);

    	for (int i = 0; i < numSockets; ++i) {
    	  if (FD_ISSET(i, &fReadSet) || FD_ISSET(i, &fWriteSet) || FD_ISSET(i, &fExceptionSet)) {
    	    fprintf(stderr, " %d(", i);
    	    if (FD_ISSET(i, &fReadSet)) fprintf(stderr, "r");
    	    if (FD_ISSET(i, &fWriteSet)) fprintf(stderr, "w");
    	    if (FD_ISSET(i, &fExceptionSet)) {fprintf(stderr, "e");gbackupErrSocket = i;}
    	    fprintf(stderr, ")");
    	  }
    	}
	    fprintf(stderr, "\n");
#endif
	//internalError();
      }
  }

  // Call the handler function for one readable socket:
  HandlerIterator iter(*fHandlers);
  HandlerDescriptor* handler;
  // To ensure forward progress through the handlers, begin past the last
  // socket number that we handled:
  if (fLastHandledSocketNum >= 0) {
    while ((handler = iter.next()) != NULL) {
      if (handler->socketNum == fLastHandledSocketNum) break;
    }
    if (handler == NULL) {
      fLastHandledSocketNum = -1;
      iter.reset(); // start from the beginning instead
    }
  }
  while ((handler = iter.next()) != NULL) {
    int sock = handler->socketNum; // alias
    int resultConditionSet = 0;
    if (FD_ISSET(sock, &readSet) && FD_ISSET(sock, &fReadSet)/*sanity check*/) resultConditionSet |= SOCKET_READABLE;
    if (FD_ISSET(sock, &writeSet) && FD_ISSET(sock, &fWriteSet)/*sanity check*/) resultConditionSet |= SOCKET_WRITABLE;
    if (FD_ISSET(sock, &exceptionSet) && FD_ISSET(sock, &fExceptionSet)/*sanity check*/) resultConditionSet |= SOCKET_EXCEPTION;
    if ((resultConditionSet&handler->conditionSet) != 0 && handler->handlerProc != NULL) {
      fLastHandledSocketNum = sock;
          // Note: we set "fLastHandledSocketNum" before calling the handler,
          // in case the handler calls "doEventLoop()" reentrantly.
      (*handler->handlerProc)(handler->clientData, resultConditionSet);
      break;
    }
  }
  if (handler == NULL && fLastHandledSocketNum >= 0) {
    // We didn't call a handler, but we didn't get to check all of them,
    // so try again from the beginning:
    iter.reset();
    while ((handler = iter.next()) != NULL) {
      int sock = handler->socketNum; // alias
      int resultConditionSet = 0;
      if (FD_ISSET(sock, &readSet) && FD_ISSET(sock, &fReadSet)/*sanity check*/) resultConditionSet |= SOCKET_READABLE;
      if (FD_ISSET(sock, &writeSet) && FD_ISSET(sock, &fWriteSet)/*sanity check*/) resultConditionSet |= SOCKET_WRITABLE;
      if (FD_ISSET(sock, &exceptionSet) && FD_ISSET(sock, &fExceptionSet)/*sanity check*/) resultConditionSet |= SOCKET_EXCEPTION;
      if ((resultConditionSet&handler->conditionSet) != 0 && handler->handlerProc != NULL) {
	fLastHandledSocketNum = sock;
	    // Note: we set "fLastHandledSocketNum" before calling the handler,
            // in case the handler calls "doEventLoop()" reentrantly.
	(*handler->handlerProc)(handler->clientData, resultConditionSet);
	break;
      }
    }
    if (handler == NULL) fLastHandledSocketNum = -1;//because we didn't call a handler
  }

  // Also handle any newly-triggered event (Note that we do this *after* calling a socket handler,
  // in case the triggered event handler modifies The set of readable sockets.)
  if (fTriggersAwaitingHandling != 0) {
    if (fTriggersAwaitingHandling == fLastUsedTriggerMask) {
      // Common-case optimization for a single event trigger:
      fTriggersAwaitingHandling &=~ fLastUsedTriggerMask;
      if (fTriggeredEventHandlers[fLastUsedTriggerNum] != NULL) {
	(*fTriggeredEventHandlers[fLastUsedTriggerNum])(fTriggeredEventClientDatas[fLastUsedTriggerNum]);
      }
    } else {
      // Look for an event trigger that needs handling (making sure that we make forward progress through all possible triggers):
      unsigned i = fLastUsedTriggerNum;
      EventTriggerId mask = fLastUsedTriggerMask;

      do {
    	i = (i+1)%MAX_NUM_EVENT_TRIGGERS;
    	mask >>= 1;
    	if (mask == 0) mask = 0x80000000;

    	if ((fTriggersAwaitingHandling&mask) != 0) {
    	  fTriggersAwaitingHandling &=~ mask;
    	  if (fTriggeredEventHandlers[i] != NULL) {
    	    (*fTriggeredEventHandlers[i])(fTriggeredEventClientDatas[i]);
    	  }

    	  fLastUsedTriggerMask = mask;
    	  fLastUsedTriggerNum = i;
    	  break;
    	}
      } while (i != fLastUsedTriggerNum);
    }
  }

  // Also handle any delayed event that may have come due.
  fDelayQueue.handleAlarm();
}

void print_backtrace(void)
{
    void *bt[16];
    int bt_size;
    char **bt_syms;
    int i;

    bt_size = backtrace(bt, 16);
    bt_syms = backtrace_symbols(bt, bt_size);
    printf("backtrace() returned %d addresses\n", bt_size);
    printf("BACKTRACE ------------\n");

    for (i = 1; i < bt_size; i++)
    {
        printf("%s\n", bt_syms[i]);
    }

    printf("----------------------\n");
    free(bt_syms);
}

#define TCP_INFO           11      /* Information about this connection. */
typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;
struct tcp_info {
	__u8	tcpi_state;
	__u8	tcpi_ca_state;
	__u8	tcpi_retransmits;
	__u8	tcpi_probes;
	__u8	tcpi_backoff;
	__u8	tcpi_options;
	__u8	tcpi_snd_wscale : 4, tcpi_rcv_wscale : 4;
	__u8	tcpi_delivery_rate_app_limited:1;

	__u32	tcpi_rto;
	__u32	tcpi_ato;
	__u32	tcpi_snd_mss;
	__u32	tcpi_rcv_mss;

	__u32	tcpi_unacked;
	__u32	tcpi_sacked;
	__u32	tcpi_lost;
	__u32	tcpi_retrans;
	__u32	tcpi_fackets;

	/* Times. */
	__u32	tcpi_last_data_sent;
	__u32	tcpi_last_ack_sent;     /* Not remembered, sorry. */
	__u32	tcpi_last_data_recv;
	__u32	tcpi_last_ack_recv;

	/* Metrics. */
	__u32	tcpi_pmtu;
	__u32	tcpi_rcv_ssthresh;
	__u32	tcpi_rtt;
	__u32	tcpi_rttvar;
	__u32	tcpi_snd_ssthresh;
	__u32	tcpi_snd_cwnd;
	__u32	tcpi_advmss;
	__u32	tcpi_reordering;

	__u32	tcpi_rcv_rtt;
	__u32	tcpi_rcv_space;

	__u32	tcpi_total_retrans;

	__u64	tcpi_pacing_rate;
	__u64	tcpi_max_pacing_rate;
	__u64	tcpi_bytes_acked;    /* RFC4898 tcpEStatsAppHCThruOctetsAcked */
	__u64	tcpi_bytes_received; /* RFC4898 tcpEStatsAppHCThruOctetsReceived */
	__u32	tcpi_segs_out;	     /* RFC4898 tcpEStatsPerfSegsOut */
	__u32	tcpi_segs_in;	     /* RFC4898 tcpEStatsPerfSegsIn */

	__u32	tcpi_notsent_bytes;
	__u32	tcpi_min_rtt;
	__u32	tcpi_data_segs_in;	/* RFC4898 tcpEStatsDataSegsIn */
	__u32	tcpi_data_segs_out;	/* RFC4898 tcpEStatsDataSegsOut */

	__u64   tcpi_delivery_rate;
};
int GetTcpInfoState(int socketnum)
{
    socklen_t len = 0;
    struct tcp_info tcpinfo;

    memset(&tcpinfo, 0, sizeof(tcpinfo));
    len = sizeof(tcpinfo);
    getsockopt(socketnum,IPPROTO_TCP,TCP_INFO,&tcpinfo,&len);

    return tcpinfo.tcpi_state;
}

void BasicTaskScheduler
  ::setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc* handlerProc, void* clientData) {
  if (socketNum < 0) return;
#if !defined(__WIN32__) && !defined(_WIN32) && defined(FD_SETSIZE)
  if (socketNum >= (int)(FD_SETSIZE)) return;
#endif
  FD_CLR((unsigned)socketNum, &fReadSet);
  FD_CLR((unsigned)socketNum, &fWriteSet);
  FD_CLR((unsigned)socketNum, &fExceptionSet);

#if 1
    if(gbackupErrSocket == socketNum && gbackupErrSocket != 0xffff){
        printf("%s:close socketNum(%d)(%d)\n",__FUNCTION__,socketNum,conditionSet);
        system("ls /proc/463/fd -l");
        //system("ls /proc/463/fd -l |grep /dev/mi_poll");
        system("cat /proc/net/tcp");
        print_backtrace();
        gbackupErrSocket = 0xffff;
    }
#endif
    
  if (conditionSet == 0) {    
    fHandlers->clearHandler(socketNum);
    if (socketNum+1 == fMaxNumSockets) {
      --fMaxNumSockets;
      #if 0
      printf("%s:-fMaxNumSockets=%d\n",__FUNCTION__,fMaxNumSockets);
      system("ls /proc/463/fd -l |grep socket");
      system("cat /proc/net/tcp");
      #endif
    }
  } else {
    fHandlers->assignHandler(socketNum, conditionSet, handlerProc, clientData);
    if (socketNum+1 > fMaxNumSockets) {
      fMaxNumSockets = socketNum+1;
      #if 0
      printf("%s:fMaxNumSockets=%d\n",__FUNCTION__,fMaxNumSockets);
      system("ls /proc/463/fd -l |grep socket");
      system("cat /proc/net/tcp");
      #endif
    }

    #if 0
    int status = GetTcpInfoState(socketNum);
    if(TCP_TIME_WAIT == status || TCP_LAST_ACK == status || TCP_CLOSE == status){        //LAST_ACK
        printf("#################WARN %d ####################\n",status);
        printf("%s:socketNum=%d\n",__FUNCTION__,socketNum);
        //system("ls /proc/463/fd -l |grep socket");
        //system("cat /proc/net/tcp");
        //print_backtrace();
        printf("##########################################\n");
    }
    #endif
    
    if (conditionSet&SOCKET_READABLE) FD_SET((unsigned)socketNum, &fReadSet);
    if (conditionSet&SOCKET_WRITABLE) FD_SET((unsigned)socketNum, &fWriteSet);
    if (conditionSet&SOCKET_EXCEPTION) FD_SET((unsigned)socketNum, &fExceptionSet);
  }
}

void BasicTaskScheduler::moveSocketHandling(int oldSocketNum, int newSocketNum) {
  if (oldSocketNum < 0 || newSocketNum < 0) return; // sanity check
#if !defined(__WIN32__) && !defined(_WIN32) && defined(FD_SETSIZE)
  if (oldSocketNum >= (int)(FD_SETSIZE) || newSocketNum >= (int)(FD_SETSIZE)) return; // sanity check
#endif
  if (FD_ISSET(oldSocketNum, &fReadSet)) {FD_CLR((unsigned)oldSocketNum, &fReadSet); FD_SET((unsigned)newSocketNum, &fReadSet);}
  if (FD_ISSET(oldSocketNum, &fWriteSet)) {FD_CLR((unsigned)oldSocketNum, &fWriteSet); FD_SET((unsigned)newSocketNum, &fWriteSet);}
  if (FD_ISSET(oldSocketNum, &fExceptionSet)) {FD_CLR((unsigned)oldSocketNum, &fExceptionSet); FD_SET((unsigned)newSocketNum, &fExceptionSet);}
  fHandlers->moveHandler(oldSocketNum, newSocketNum);

  if (oldSocketNum+1 == fMaxNumSockets) {
    --fMaxNumSockets;
    printf("%s:-fMaxNumSockets=%d\n",__FUNCTION__,fMaxNumSockets);
    system("ls /proc/463/fd -l |grep socket");
    system("ls /proc/463/fd -l |grep /dev/mi_poll");
    system("cat /proc/net/tcp");
  }
  if (newSocketNum+1 > fMaxNumSockets) {
    fMaxNumSockets = newSocketNum+1;
    printf("%s:fMaxNumSockets=%d\n",__FUNCTION__,fMaxNumSockets);
    system("ls /proc/463/fd -l |grep socket");
    system("cat /proc/net/tcp");
  }

    #if 1
    int status = GetTcpInfoState(newSocketNum);
    if(TCP_TIME_WAIT == status || TCP_LAST_ACK == status || TCP_CLOSE == status){        //LAST_ACK
        printf("#################WARN %d ####################\n",status);
        printf("%s:socketNum=%d\n",__FUNCTION__,newSocketNum);
        system("ls /proc/463/fd -l |grep socket");
        system("ls /proc/463/fd -l |grep /dev/mi_poll");
        system("cat /proc/net/tcp");
        printf("##########################################\n");
    }
    #endif
}
