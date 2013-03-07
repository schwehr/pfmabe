/********************************************************************
 *
 * Module Name : SNPTIME.H
 *
 * Author/Date : J. S. Byrne / 5 Nov 1993
 *
 * Description : This header file contains the interface definitions for
 *     the snptime library.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who	when	 what
 * ---	----	 ----
 * jsb	5/14/94  changed definition of IRIG clock to CLOCK_IRIGB
 *
 * Classification : Unclassified
 *
 * References : Posix P1003.4 Draft Standard
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/

#ifdef __cplusplus
      extern "C" {
#endif


/*
 * Standard header file for time support library
 */
#ifndef    _SNP_TIME_H_
   #define _SNP_TIME_H_

   /* We need ansi time support */
   #include <time.h>

   /* define Posix.4 proposed structure for internal storage of time */
#if !defined(_STRUCT_TIMESPEC_) && !defined (_STRUCT_TIMESPEC) && !defined (__MINGW64__)	/* SAIC define */
       #define _STRUCT_TIMESPEC_
       #define _STRUCT_TIMESPEC
       struct timespec
       {
          time_t        tv_sec;
          long	        tv_nsec;
       };
   #endif

   /*
    * timeval defined by IBM TCPIP in /tcpip/include/sys/time.h
    *	and defined by HPUX	 in /usr/include/sys/time.h
    */
   #ifndef __MINGW64__
     #ifndef    _STRUCT_TIMEVAL	     /* hpux define */
        #define _STRUCT_TIMEVAL
        #ifndef  _TIMEVAL 	 /* IBM TCPIP define */
         struct timeval {
	   unsigned long	tv_sec;
	   unsigned long	tv_usec;
         };
        #endif
     #endif
   #endif

   /*
    * Define a structure which is expected to be in shared memory on an OS/2
    *  machine and used to bias the PC clock to obtain a "precise" time.
    */
   typedef struct _snp_clock_delta {
      int sign;
      int seconds;
      int nanosec;
   } SNP_CLOCK_DELTA;

   /* Now define the name of the shared memory area written to */
   #define CLOCK_DELTA_SHARED	  "\\SHAREMEM\\CLOCK_DELTA_SHARED"

   /*
    * Clock definitions
    */
   #define CLOCK_REALTIME  1L	/* The clock maintained by local computer */
   #define CLOCK_SNP_IRIGB 2L	/* The IRIG B time distribution network   */

   /*
    * Errno definitions for the Posix functions
    */
   #ifndef  EINVAL
      #define EINVAL 1	    /* unknown clock_id, or bad time specified	    */
   #endif
   #ifndef  EPERM
      #define EPERM  2	    /* insufficient privilege to set specified clock   */
   #endif
   #ifndef  ENOSYS
      #define ENOSYS 3	    /* clock, or function not supported on this system */
   #endif

   #ifndef  TRUE
    #define TRUE 1
   #endif
   #ifndef  FALSE
    #define FALSE 0
   #endif

   /*
    * Some useful time manipulation MACROS:
    *  TIMESPEC_TO_ANSI(tp,ansi,nano)  converts a struct timespec to an ansi time_t and nanoseconds
    *  ANSI_TO_TIMESPEC(ansi,nano,tp)  converts an ansi time_t and nanoseconds to a struct timespec
    *  TIMESPEC_TO_TIMEVAL(tspec,tval) converts a struct timespec to a struct timeval
    *  TIMEVAL_TO_TIMESPEC(tval,tspec) converts a sturct timeval to a struct timespec
    *  DIFFERENT_TIME(tp1,tp2)	  true if timespecs tp1 is equal to timespec tp2
    *  EQUATE_TIMES(tp1,tp2)	  sets timespec tp2 equal to the timespec tp1
    *  MJD(m,i,k)	 computes the modified julian day from the month day and year
    *  SET_TZ_GMT	 sets the TZ environment variable to GMT
    */
   #ifndef  TIMESPEC_TO_ANSI
      #define TIMESPEC_TO_ANSI(tp, ansi, nano) (ansi = tp.tv_sec);(nano = tp.tv_nsec)
   #endif

   #ifndef  ANSI_TO_TIMESPEC
      #define ANSI_TO_TIMESPEC(ansi, nano, tp) (tp.tv_sec = ansi);(tp.tv_nsec = nano)
   #endif

   #ifndef  TIMESPEC_TO_TIMEVAL
      #define TIMESPEC_TO_TIMEVAL(tspec,tval) (tval.tv_sec = tspec.tv_sec);(tval.tv_nsec = tspec.tv_nsec/1.0e3)
   #endif

   #ifndef  TIMEVAL_TO_TIMESPEC
      #define TIMEVAL_TO_TIMESPEC(tval,tspec) (tspec.tv_sec = tval.tv_sec);(tspec.tv_nsec = tval.tv_usec*1.0e3)
   #endif

   #ifndef  DIFFERENT_TIME
      #define DIFFERENT_TIME(tp1,tp2) (memcmp(&tp1, &tp2, sizeof(tp1)) ? TRUE : FALSE)
   #endif

   #ifndef  EQUATE_TIMES
      #define EQUATE_TIMES(tp1,tp2) (memcpy(&tp2, &tp1, sizeof(tp1)))
   #endif

   #ifndef  ZEROTIME
      #define ZEROTIME(tp1) (tp1.tv_sec=0);(tp1.tv_nsec=0)
   #endif

   #ifndef  MJD
      #define MJD(m,i,k) ((367.0*(k))-((7*((k)+(((m)+9)/12)))/4)+((275*(m))/9+(i))-678987.0)
   #endif

   #ifndef  SET_TZ_GMT
      #define SET_TZ_GMT putenv("TZ=GMT"); tzset()
   #endif

   /*
    * Function prototypes
    */
#ifdef OS2
   int	   _Optlink clock_settime (clock_t clock_id, struct timespec *tp);
   int	   _Optlink clock_gettime (clock_t clock_id, struct timespec *tp);
   int	   _Optlink clock_getres (clock_t clock_id, double *res);
   int	   _Optlink DateTimeToInternalTime (double date, double time, struct timespec *tp);
   int	   _Optlink InternalTimeToDateTime (struct timespec *tp, double *date, double *time);
   void    _Optlink AddTimes (struct timespec *base_time, double delta_time, struct timespec *sum_time);
   void    _Optlink SubtractTimes (struct timespec *base_time, struct timespec *subtrahend, double *delta_time);
   int	   _Optlink TimeLater (struct timespec *tp1, struct timespec *tp2);
   int	   _Optlink JulianDay (int month, int day, int year);
   void    _Optlink mdy (double i, int *m, int *d, int *y);
   int	   _Optlink JulianDayToMonthDay (int year, int j_day, int *month, int *day);
   int	   _Optlink crossover (double t1, double t2, double dt);
   /*
    * The following function is required by clock_gettime. It is provided for
    *  library users to link to in a hardware specific obj called aitg_gpt.  A
    *  dummy obj, (dumm_gpt.obj) is provided for users of the time library who
    *  don't have access to the hardware specific capability provided in
    *  aitg_gpt. If you specify the clock CLOCK_IRIG_B, in a call to
    *  clock_gettime, and you've linked to dumm_gpt.obj, you will get back a
    *  time from the CLOCK_WORKSTATION clock, and a return code of -1.
    */
   int _Optlink GetPreciseTime (struct timespec *tp);
#else
   #ifndef    _STRUCT_TIMEVAL	     /* hpux define */
        int	   clock_settime (clock_t clock_id, struct timespec *tp);
        int	   clock_gettime (clock_t clock_id, struct timespec *tp);
        int	   clock_getres (clock_t clock_id, double *res);
   #endif
   int	   DateTimeToInternalTime (double date, double time, struct timespec *tp);
   int	   InternalTimeToDateTime (struct timespec *tp, double *date, double *time);
   void    AddTimes (struct timespec *base_time, double delta_time, struct timespec *sum_time);
   void    SubtractTimes (struct timespec *base_time, struct timespec *subtrahend, double *delta_time);
   int	   TimeLater (struct timespec *tp1, struct timespec *tp2);
   int	   JulianDay (int month, int day, int year);
   void    mdy (double i, int *m, int *d, int *y);
   int	   JulianDayToMonthDay (int year, int j_day, int *month, int *day);
   int	   crossover (double t1, double t2, double dt);
#endif

#endif


#ifdef __cplusplus
      }
#endif
