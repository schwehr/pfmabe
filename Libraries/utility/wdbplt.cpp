
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include "nvmap.hpp"

#define         PHYSIZ      3072
 
#define         SIGN_OF(x)  ((x)<0.0 ? -1 : 1)
 
static FILE             *g_lunfil;
static NV_U_CHAR        g_bytbuf[PHYSIZ], g_celbuf[PHYSIZ];
static NV_INT32         g_logrec, g_lperp, g_offset, g_level;
static NV_CHAR          g_stflag;
static NV_INT32         g_index, g_addr, g_paddr, g_lstat, g_curpos, g_fulrec, 
                        g_size;
static NV_FLOAT64       g_slatdd, g_nlatdd, g_wlondd, g_elondd;

static NV_INT32         flag, pcaddr, npts, prank;

/***************************************************************************/
/*! <pre>

*      Function wdbplt                                                      *
*****************************************************************************
*      Retrieves the WDB II or WVS data from the direct access files.       *
*                                                                           *
*                                                                           *
*                                                                           *
*       Authors :                                                           *
*                                                                           *
*         Jan C. Depner                       James A. Hammack              *
*         U.S. Naval Oceanographic Office     NORDA                         *
*         Information Systems Department      Code 117                      *
*         Stennis Space Center, MS            Stennis Space Center, MS      *
*         39522-5001                          39529-5004                    *
*                                                                           *
*       C version : Jan C. Depner                                           *
*       Date : 02-24-89                                                     *
*                                                                           *
*       NOTE : It should be intuitively obvious to the most casual observer *
*              that I ain't no C programmer.  This is my first effort and,  *
*              hopefully, these routines will be good enough to get the job *
*              done.  (JCD)                                                 *
*                                                                           *
*       Global variable definitions :                                       *
*                                                                           *
*       PHYSIZ    - size of a physical record in the direct access file,    *
*                   in bytes.                                               *
*       g_bytbuf  - char array of 'PHYSIZ' length for i/o to the direct     *
*                   access file.                                            *
*       g_celbuf  - char array of 'PHYSIZ' length for i/o to the            *
*                   direct access file (used to retrieve cell map).         *
*       g_lunfil  - file handle for the direct access file.                 *
*       g_logrec  - length of a logical record in bytes.                    *
*       g_lperp   - logical records per physical record.                    *
*       g_offset  - number of logical records needed to store the bit       *
*                   cell map + 1 .                                          *
*       g_level   - wdbplt software and data file version.                  *
*       g_proj    - projection (6 chars + 0x0).                             *
*       g_stflag  - initialization flag for 'prject' and 'curve'.           *
*       g_index   - cell index number for algorithm addressing, g_index =   *
*                   integer (lat degrees + 90) * 360 + integer lon          *
*                   degrees + 180 + 1 ; this is the logical record          *
*                   address of the first data segment in any cell.          *
*       g_addr    - current physical record address in bytes from beginning *
*                   of the file.                                            *
*       g_paddr   - previous physical record address.                       *
*       g_lstat   - seek and read status variable.                          *
*       g_curpos  - current byte position within the physical record.       *
*       g_fulrec  - full record value (g_logrec-4).                         *
*       g_slatdd  - latitude of southern boundary of area (degrees).        *
*       g_nlatdd  - latitude of northern boundary of area (degrees).        *
*       g_wlondd  - longitude of western boundary of area (degrees).        *
*       g_elondd  - longitude of eastern boundary of area (degrees).        *
*       g_width   - width of area.                                          *
*       g_height  - height of area.                                         *
*                                                                           *
*       Local variable definitions :                                        *
*                                                                           *
*       ioff      - divisor for delta records (1 - WDB, 10 - WVS).          *
*       i         - utility variable.                                       *
*       j         - utility variable.                                       *
*       col       - longitude index.                                        *
*       go        - beginning value for longitude loop.                     *
*       stop      - ending value for longitude loop.                        *
*       inc       - increment for longitude loop.                           *
*       slat      - integer value of southern latitude of area (degrees).   *
*       nlat      - integer value of northern latitude of area (degrees).   *
*       wlon      - integer value of western longitude of area (degrees).   *
*       elon      - integer value of eastern longitude of area (degrees).   *
*       slatf     - integer value of southern latitude of file (degrees).   *
*       nlatf     - integer value of northern latitude of file (degrees).   *
*       wlonf     - integer value of western longitude of file (degrees).   *
*       elonf     - integer value of eastern longitude of file (degrees).   *
*       widef     - integer width of file in degrees.                       *
*       size      - number of cells in the input file (for subset file).    *
*       segcnt    - number of data points in the current segment.           *
*       rank      - type of data of the current segment.                    *
*       cont      - segment continuation flag.                              *
*       cnt       - segment loop counter.                                   *
*       gap       - every 'gap'th point will be plotted; largest allowed    *
*                   value for gap is 45.                                    *
*       latsec    - delta latitude in seconds or tenths of seconds.         *
*       lonsec    - delta longitude in seconds or tenths of seconds.        *
*       latoff    - latitude offset (seconds or tenths of seconds).         *
*       lonoff    - longitude offset (seconds or tenths of seconds).        *
*       conbyt    - byte position of continuation pointer within physical   *
*                   record.                                                 *
*       eflag     - end of segment flag.                                    *
*       first     - initialization flag.                                    *
*       filenm    - full filename of data file.                             *
*       todeg     - conversion factor from deltas to degrees.               *
*       lat       - latitude of current point (degrees +90.0).              *
*       lon       - longitude of current point (degrees).                   *
*       dummy     - dummy pointer for modf function.                        *
*                                                                           *
*       Arguments:                                                          *
*                                                                           *
*       file   - 7 characters, 6 character file identifier & 0x0.           *
*       slatd  - latitude of southern boundary of area.                     *
*       nlatd  - latitude of northern boundary of area.                     *
*       wlond  - longitude of western boundary of area.                     *
*       elond  - longitude of eastern boundary of area.                     *
*       map    - map object to be plotted on                                *
*       color  - line color                                                 *
*       width  - line width                                                 *
*                                                                           *
</pre>
****************************************************************************/

void wdbplt(NV_CHAR *file, NV_FLOAT64 slatd, NV_FLOAT64 nlatd, NV_FLOAT64 wlond, 
            NV_FLOAT64 elond, nvMap *map, QColor color, NV_INT32 line_width)
{
    NV_INT32 celchk (NV_INT32);
    void nxtrec ();
    void movpos ();
    void pltpro (nvMap *, QColor, NV_INT32, NV_FLOAT64, NV_FLOAT64, NV_INT32, 
                 NV_INT32 *, NV_INT32, NV_FLOAT64);
 
    NV_INT32   ioff, i, j, col, go, stop, inc, slat, nlat, wlon, elon, segcnt,
               rank, cont, cnt, gap, latsec, lonsec, slatf, nlatf, wlonf,
               elonf, widef, first, areaflag, eflag;
    NV_INT32   latoff, lonoff, conbyt, lats, lons;
    NV_CHAR    dirfil[132];
    NV_FLOAT64 todeg, lat, lon, lnbias;
    NV_FLOAT64 dummy;


    /*  Initialize variables, open file and read first record.  */
 
    first = 1;
    eflag = 0;
    g_stflag = 1;
    flag = 1;
    pcaddr = 0;
    npts = 0;
    prank = 0;
    g_paddr = -1;
    lats = 0;
    lons = 0;
    rank = 0;
    

    /*  Use the environment variable WVS_DIR to get the         */
    /*  directory name.                                         */
    /*                                                          */
    /*  To set the variable in csh use :                        */
    /*                                                          */
    /*      setenv WVS_DIR /usr/wdbii                           */
    /*                                                          */
    /*  To set the variable in sh or ksh use :                  */
    /*                                                          */
    /*      WVS_DIR=/usr/wdbii                                  */
    /*      export WVS_DIR                                      */


    if (getenv ("WVS_DIR") == NULL)
    {
      fprintf (stderr, "\n\nEnvironment variable WVS_DIR is not set\n\n");
      fflush (stderr);
      return;
    }
    
    sprintf (dirfil, "%s%1c%s", getenv ("WVS_DIR"), (NV_CHAR) SEPARATOR, file);

    g_lunfil = fopen (dirfil, "rb");
    if (g_lunfil == NULL)
    {
        perror (file);
        return;
    }
    g_lstat = fseek (g_lunfil, 0, 0);
    g_lstat = fread (g_bytbuf, 3072, 1, g_lunfil);
    g_logrec = g_bytbuf[3];
    g_fulrec = g_logrec - 4;
    g_level = g_bytbuf[4];
    ioff = g_bytbuf[5];
    slatf = g_bytbuf[6] * 256 + g_bytbuf[7];
    nlatf = g_bytbuf[8] * 256 + g_bytbuf[9];
    wlonf = g_bytbuf[10] * 256 + g_bytbuf[11];
    elonf = g_bytbuf[12] * 256 + g_bytbuf[13];
    if (elonf < wlonf) elonf += 360;
    if (slatf + nlatf + wlonf + elonf == 0)
    {
        nlatf = 180;
        elonf = 360;
    }
    widef = elonf - wlonf;
    g_size = (nlatf - slatf) * (NV_INT32)widef;
    todeg = 3600.0 * ioff;
    g_offset = (g_size - 1) / (g_logrec*8) + 2;
    g_lperp = PHYSIZ / g_logrec;

    /*  Adjust gapin if necessary.                              */
    
    gap = 1;
    if (gap >= 45)
    {
        fprintf (stderr, "Plot gap is too large!    ");
        fprintf (stderr, "It has been reset to 45.\n");
        fflush (stderr);
        gap = 45;
    }
    if (gap<=0)
    {
        fprintf (stderr, "Plot gap is 0 or negative!    ");
        fprintf (stderr, "It has been reset to 1.\n");
        fflush (stderr);
        gap = 1;
    }

    /*  Compute latitude and longitude in degrees and adjust    */
    /*  for 180 crossing                                        */

    g_slatdd = slatd;
    g_nlatdd = nlatd;
    g_wlondd = wlond;
    lnbias = g_wlondd;
    g_elondd = elond;
    if (g_wlondd < 0.0) g_wlondd = g_wlondd + 360.0;
    if (g_elondd < 0.0) g_elondd = g_elondd + 360.0;
    if (g_elondd <= g_wlondd) g_elondd = g_elondd + 360.0;
    lnbias = lnbias - g_wlondd;

    /*  Compute start and end integer values for retrieval      */
    /*  loop and adjust if necessary.                           */
    
    slat = (int) (g_slatdd + 90.0);
    nlat = (int) (g_nlatdd + 90.0);
    wlon = (int) g_wlondd;
    elon = (int) g_elondd;
 
    if (modf (g_nlatdd, &dummy) == 0.0) nlat--;
    if (modf (g_elondd, &dummy) == 0.0) elon--;

    /*  Latitude loop.                                          */
    
    for (i = slat ; i <= nlat ; i++)
    {

        /*  Change direction of retrieval to minimize movement  */
        /*  on mechanical plotters.                             */
        
        if (i % 2 == 0)
        {
            go = wlon;
            stop = elon;
            inc = 1;
        }
        else
        {
            go = elon;
            stop = wlon;
            inc = -1;
        }

        /*  Longitude loop.                                     */
        
        for (j = go ; j * inc <= stop * inc ; j += inc)
        {

        /*  Use latitude and longitude loop counters to compute */
        /*  index into direct access data base.                 */
        
            col = j % 360;
            if (col < -180) col = col + 360;
            if (col >= 180) col = col - 360;
            col = col +180;
            if (col < wlonf) col += 360;
            g_index = (i - slatf) * (NV_INT32) widef + (col - wlonf) + 1 +
            g_offset;

            /*  Check for cell outside of file area.            */
            
            if (i < slatf || i >= nlatf || col < wlonf || col >= elonf)
            {
                areaflag = 0;
            }
            else
            {
                areaflag = 1;
            }

            /*  Check cell map to see if data is available in   */
            /*  'g_index' cell.                                 */
            
            if (areaflag && celchk (first))
            {

                /*  Compute physical record address, read       */
                /*  record and save as previous address.        */
                
                eflag = 0;
                g_addr = ((g_index - 1) / g_lperp) * PHYSIZ;
                if (g_addr != g_paddr)
                {
                    g_lstat = fseek (g_lunfil, g_addr, 0);
                    g_lstat = fread (g_bytbuf, PHYSIZ, 1, g_lunfil);
                }
                g_paddr = g_addr;

                /*  Compute byte position within physical       */
                /*  record.                                     */
                
                g_curpos = ((g_index - 1) % g_lperp) * g_logrec;

                /*  If not at end of segment, process the       */
                /*  record.                                     */
                
                while (!eflag)
                {

                    /*  Get first two bytes of header and break */
                    /*  out count and continuation bit.         */
                    
                    segcnt = (g_bytbuf[g_curpos] % 128) * 4 +
                        g_bytbuf[g_curpos + 1] / 64 + 1;
                    cont = g_bytbuf[g_curpos] / 128;

                    /*  If this is a continuation record get    */
                    /*  offsets from the second byte.           */
                    
                    if (cont)
                    {
                        latoff = ((g_bytbuf[g_curpos + 1] % 64) / 8) *
                            (NV_INT32) 65536;
                        lonoff = (g_bytbuf[g_curpos + 1] % 8) *
                            (NV_INT32) 65536;
                    }

                    /*  If this is an initial record set the    */
                    /*  offsets to zero and get the rank from   */
                    /*  the second byte.                        */
                    
                    else
                    {
                        latoff = 0;
                        lonoff = 0;
                        rank = g_bytbuf[g_curpos + 1] % 64;
                    }

                    /*  Update the current byte position and    */
                    /*  get a new record if necessary.          */
                    
                    movpos ();

                    /*  Compute the rest of the latitude        */
                    /*  offset.                                 */
                    
                    latoff += g_bytbuf[g_curpos] * (NV_INT32) 256 +
                        g_bytbuf[g_curpos + 1];
 
                    movpos ();

                    /*  Compute the rest of the longitude       */
                    /*  offset.                                 */
                    
                    lonoff += g_bytbuf[g_curpos] * (NV_INT32) 256 +
                        g_bytbuf[g_curpos + 1];

                    /*  If this is a continuation record, bias  */
                    /*  the lat and lon offsets and compute the */
                    /*  position.                               */
                    
                    if (cont)
                    {
                        latoff -= 262144;
                        lonoff -= 262144;
                        lats += latoff;
                        lons += lonoff;
                    }

                    /*  Else, compute the position.             */
                    
                    else
                    {
                        lats = (int) (i * todeg + latoff);
                        lons = (int) (j * todeg + lonoff);
                    }
                    lat = (NV_FLOAT64) lats / todeg;
                    lon = (NV_FLOAT64) lons / todeg;

                    /*  Update the current byte position.       */
                    
                    g_curpos += 2;

                    /*  Get the continuation pointer.           */
                    
                    conbyt = ((g_index-1) % g_lperp) * g_logrec + g_fulrec;

                    /*  If there is no continuation pointer or  */
                    /*  the byte position is not at the         */
                    /*  position pointed to by the continuation */
                    /*  pointer, process the segment data.      */
                    
                    if (g_bytbuf[conbyt] == 0 || (g_curpos + 1) %
                        g_logrec <= g_bytbuf[conbyt])
                    {

                        /*  If at the end of the logical        */
                        /*  record, get the next record in the  */
                        /*  chain.                              */
                        
                        if (g_curpos % g_logrec == g_fulrec &&
                            g_bytbuf[conbyt] == 0) nxtrec ();

                        /*  If the rank is to be plotted call   */
                        /*  the plot routine.                   */
                        
                        pltpro (map, color, line_width, lat, lon, rank, &cont, 
                                gap, lnbias);

                        /*  If the end of the segment has been  */
                        /*  reached, set the end flag.          */
                        
                        if ((g_curpos + 1) % g_logrec == g_bytbuf[conbyt])
                            eflag = 1;

                        /*  Process the segment.                */
                        
                        for (cnt = 2 ; cnt <= segcnt ; cnt++)
                        {

                            /*  Compute the position from the   */
                            /*  delta record.                   */
                            
                            latsec = g_bytbuf[g_curpos] - 128;
                            lats += latsec;
                            lat = (NV_FLOAT64) lats / todeg;
                            lonsec = g_bytbuf[g_curpos + 1] - 128;
                            lons += lonsec;
                            lon = (NV_FLOAT64) lons / todeg;

                            /*  Call the plotting routine.      */
                            
                            pltpro (map, color, line_width, lat, lon, rank, &cont, 
                                    gap, lnbias);
 
                            g_curpos += 2;
 
                            conbyt = ((g_index-1) % g_lperp) *
                                g_logrec + g_fulrec;

                            /*  If the end of the segment has   */
                            /*  been reached, set the end flag  */
                            /*  and break out of for loop.      */
                            
                            if ((g_curpos + 1) % g_logrec ==
                                g_bytbuf[conbyt])
                            {
                                eflag = 1;
                                break;
                            }
                            else
                            {
                                if (g_curpos % g_logrec == g_fulrec)
                                    nxtrec ();
                            }
                        }
                    }

                    /*  Break out of while loop if at the end   */
                    /*  of the segment.                         */
                    
                    else
                    {
                        break;
                    }                           /*  end if      */
                }                               /*  end while   */
            }                                   /*  end if      */
        }                                       /*  end for     */
    }                                           /*  end for     */

    /*  Call the plot routine to flush the buffers.             */
    
    pltpro (map, color, line_width, 999.0, 999.0, 64, &cont, gap, lnbias);
    fclose (g_lunfil);


    //  Flush the coastline to the screen.

    map->flush ();
}
 
/***************************************************************************/
/*! <pre>

*       Function celchk                                                     *
*****************************************************************************
*                                                                           *
*       Checks for data in a given one-degree cell, reads bit map.          *
*                                                                           *
*       Authors :                                                           *
*                                                                           *
*         Jan C. Depner                       James A. Hammack              *
*         U.S. Naval Oceanographic Office     NORDA                         *
*         Information Systems Department      Code 117                      *
*         Stennis Space Center, MS            Stennis Space Center, MS      *
*         39522-5001                          39529-5004                    *
*                                                                           *
*       C version : Jan C. Depner                                           *
*       Date : 02-24-89                                                     *
*                                                                           *
*       Variable definitions :                                              *
*                                                                           *
*       caddr  - current physical record address in bytes from beginning    *
*                of the file (cell map address).                            *
*       pcaddr - previous physical record address (cell map address).       *
*       ndxpos - bit position within the 64800 bit cell map for the cell    *
*                pointed to by 'g_index'.                                   *
*       bytpos - byte position within the cell map of the 'g_index' cell.   *
*       bitpos - bit position within the 'bytpos' byte of the 'g_index' cell*
*                bit.                                                       *
*       chk    - logical value returned (true if there is data in the       *
*                'g_index' cell).                                           *
*                                                                           *
*       Arguments:                                                          *
*                                                                           *
*       first  - logical value that is set true the first time the cell map *
*                is accessed.                                               *
*                                                                           *
</pre>
****************************************************************************/

NV_INT32 celchk(NV_INT32 first)
{
    static NV_INT32   caddr;
    NV_INT32          ndxpos;
    NV_INT32          bytpos, bitpos, chk;

    NV_INT32 test_bit (NV_U_CHAR, NV_INT32);


    /*  Compute the physical address of the 'g_index' cell bit. */
    
    caddr = (((g_index + g_logrec * 8) - (g_offset + 1)) /
        (PHYSIZ * 8)) * PHYSIZ;

    /*  If this is the first access or the physical address has */
    /*  changed since the last access, read a new physical      */
    /*  record.                                                 */
    
    if (first || pcaddr != caddr)
    {
        g_lstat = fseek (g_lunfil, caddr, 0);
        g_lstat = fread (g_celbuf, PHYSIZ, 1, g_lunfil);
    }

    /*  Set the previous address to the current one, set        */
    /*  'first' false.                                          */
    
    pcaddr = caddr;
    first = 0;

    /*  Compute the 'g_index' position within the physical      */
    /*  record.                                                 */
    
    ndxpos = ((g_index + g_logrec * 8) - (g_offset + 1)) % (PHYSIZ * 8);

    /*  Compute the byte and bit positions.                     */
    
    bytpos = ndxpos / 8;
    bitpos = 7 - ndxpos % 8;

    /*  Test the 'g_index' bit and return.                      */
    
    chk = test_bit (g_celbuf[bytpos], bitpos);
    return (chk);
}
 
/***************************************************************************/
/*! <pre>
*       Function test_bit                                                   *
*****************************************************************************
*                                                                           *
*       Checks for bit set in an unsigned char.                             *
*                                                                           *
*       Authors :                                                           *
*                                                                           *
*         Jan C. Depner                       James A. Hammack              *
*         U.S. Naval Oceanographic Office     NORDA                         *
*         Information Systems Department      Code 117                      *
*         Stennis Space Center, MS            Stennis Space Center, MS      *
*         39522-5001                          39529-5004                    *
*                                                                           *
*       C version : Jan C. Depner                                           *
*       Date : 02-24-89                                                     *
*                                                                           *
*       Variable definitions :                                              *
*                                                                           *
*       mask  - char array of bit masks                                     *
*                                                                           *
</pre>
****************************************************************************/

NV_INT32 test_bit (NV_U_CHAR byte, NV_INT32 bitpos)
{
    static NV_U_CHAR mask[9] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
 
    return (byte & mask[bitpos]);
}
 
/***************************************************************************/
/*! <pre>
*       Function movpos                                                     *
*****************************************************************************
*                                                                           *
*       Updates current position pointer and checks for end of record.      *
*                                                                           *
*       Authors :                                                           *
*                                                                           *
*         Jan C. Depner                       James A. Hammack              *
*         U.S. Naval Oceanographic Office     NORDA                         *
*         Information Systems Department      Code 117                      *
*         Stennis Space Center, MS            Stennis Space Center, MS      *
*         39522-5001                          39529-5004                    *
*                                                                           *
*       C version : Jan C. Depner                                           *
*       Date : 02-24-89                                                     *
*                                                                           *
</pre>
****************************************************************************/

void movpos ()
{
    void nxtrec ();
 
    g_curpos += 2;
    if (g_curpos % g_logrec == g_fulrec) nxtrec ();
}
 
/***************************************************************************/
/*! <pre>
*       Function nxtrec                                                     *
*****************************************************************************
*                                                                           *
*       Reads next record in overflow chain.                                *
*                                                                           *
*       Authors :                                                           *
*                                                                           *
*         Jan C. Depner                       James A. Hammack              *
*         U.S. Naval Oceanographic Office     NORDA                         *
*         Information Systems Department      Code 117                      *
*         Stennis Space Center, MS            Stennis Space Center, MS      *
*         39522-5001                          39529-5004                    *
*                                                                           *
*       C version : Jan C. Depner                                           *
*       Date : 02-24-89                                                     *
*                                                                           *
</pre>
****************************************************************************/

void nxtrec ()
{

    /*  Compute the index number for the next logical record    */
    /*  in the chain.                                           */
    
    g_index = g_bytbuf[g_curpos + 1] * (NV_INT32) 65536 +
        g_bytbuf[g_curpos + 2] * (NV_INT32) 256 + g_bytbuf[g_curpos + 3];
    g_addr = ((g_index - 1) / g_lperp) * PHYSIZ;

    /*  If the physical record has changed since the last       */
    /*  access, read a new physical record.                     */
    
    if (g_addr != g_paddr)
    {
        g_lstat = fseek (g_lunfil, g_addr, 0);
        g_lstat = fread (g_bytbuf, PHYSIZ, 1, g_lunfil);
    }

    /*  Set the previous physical address to the current one,   */
    /*  and compute the current byte position for the new       */
    /*  record.                                                 */
    
    g_paddr = g_addr;
    g_curpos = ((g_index - 1) % g_lperp) * g_logrec;
}
 
/***************************************************************************/
/*! <pre>
*       Function pltpro                                                     *
*****************************************************************************
*                                                                           *
*       Plots the data retrieved by wdbplt.                                 *
*                                                                           *
*       Authors :                                                           *
*                                                                           *
*         Jan C. Depner                       James A. Hammack              *
*         U.S. Naval Oceanographic Office     NORDA                         *
*         Information Systems Department      Code 117                      *
*         Stennis Space Center, MS            Stennis Space Center, MS      *
*         39522-5001                          39529-5004                    *
*                                                                           *
*       C version : Jan C. Depner                                           *
*       Date : 02-24-89                                                     *
*                                                                           *
*       Variable definitions :                                              *
*                                                                           *
*       flag   - set if segment has gone outside of plotting area.          *
*       npts   - number of points in the current segment prior to 'simple'. *
*       prank  - previous rank value.                                       *
*       count  - number of points after 'simple'.                           *
*       savlat - variable used to save the 512th latitude value if the      *
*                segment is larger than 512 points.                         *
*       savlon - variable used to save the 512th longitude value if the     *
*                segment is larger than 512 points.                         *
*       latray - array of latitude values.                                  *
*       lonray - array of longitude values.                                 *
*       ipen   - pseudo pen-up/pen-down value, set based on flag and cont.  *
*       ylat   - latitude in range -90 to +90.                              *
*       i      - utility variable.                                          *
*                                                                           *
*       Arguments:                                                          *
*                                                                           *
*       lat    - latitude of current point (degrees +90.0)                  *
*       lon    - longitude of current point (degrees)                       *
*       rank   - rank of data point (only relevant for WDB data).           *
*       cont   - segment continuation flag.                                 *
*       penclr - number of color to be used for this rank.                  *
*       gap    - every 'gap'th point will be plotted; largest allowed       *
*                value for gap is 45                                        *
*                                                                           *
</pre>
****************************************************************************/

/*ARGSUSED*/
void pltpro (nvMap *map, QColor color, NV_INT32 line_width, NV_FLOAT64 lat, 
             NV_FLOAT64 lon, NV_INT32 rank, NV_INT32 *cont, 
             NV_INT32 gap, NV_FLOAT64 lnbias)
{
    NV_INT32             i;
    static NV_INT32      count;
    static NV_FLOAT64    savlat, savlon, latray[1024], lonray[1024];
    NV_CHAR              ipen;
    NV_FLOAT64           ylat;

    void simple (NV_FLOAT64 *, NV_FLOAT64 *, NV_INT32 *, NV_INT32);

    /*  Adjust latitude back to -90 to +90 range.               */
    
    ylat = lat - 90.0;

    /*  Check for beginning of line.                            */
    
    if (flag && *cont)
    {
        ipen = 2;
    }
    else
    {
        ipen = 3;
    }

    /*  If this is beginning of a line or rank has changed or   */
    /*  1024 points have been stored, plot the line.            */
    
    if (ipen == 3 || rank != prank || npts == 1024)
    {
        /*  Make sure there are at least two points in the      */
        /*  arrays.                                             */
        
        if (npts > 0)
        {
            /*  Store the count and simplify the arrays if      */
            /*  requested.                                      */
            
            count = npts;
            simple (lonray, latray, &count, gap);


            
            for (i = 1 ; i < count ; i++)
            {
              map->drawLine (lonray[i - 1], latray[i - 1], lonray[i], 
                             latray[i], color, line_width, NVFalse, Qt::SolidLine);
            }

            /*  Save the last lat and lon if the segment is     */
            /*  larger than 1024 points.                        */
            
            if (npts == 1024)
            {
                savlat = latray[1023];
                savlon = lonray[1023];
            }
        }

        /*  For line larger than 1024 points, store the 1024th  */
        /*  point in the beginning of the next line and set the */
        /*  counter.                                            */
        
        if (npts == 1024 && *cont)
        {
            latray[0] = savlat;
            lonray[0] = savlon;
            npts = 1;
        }
        else
        {
            npts = 0;
        }
    }

    /*  If the current point is within the area, store it and   */
    /*  set the flag.                                           */
    
    if (ylat >= g_slatdd && ylat <= g_nlatdd && lon >= g_wlondd &&
        lon <= g_elondd)
    {
        latray[npts] = ylat;
        lonray[npts] = lon + lnbias;
        npts++;
        flag = 1;
    }
    else
    {
        flag = 0;
    }
    *cont = -1;
    prank = rank;

    /*  If this is the last point read for this area, reset the */
    /*  'prject' initialization flag.                           */
    
    if (rank == 64) g_stflag = 1;
}

/***************************************************************************/
/*! <pre>
*       Function simple                                                     *
*****************************************************************************
*                                                                           *
*       Nth point simplification routine.                                   *
*                                                                           *
*       Author :                                                            *
*                                                                           *
*         James A. Hammack                                                  *
*         Naval Ocean Research and Development Activity                     *
*         Code 117                                                          *
*         Stennis Space Center, MS 39529-5004                               *
*                                                                           *
*       C version : Jan C. Depner                                           *
*       Date : 02-24-89                                                     *
*                                                                           *
*       Variable definitions :                                              *
*                                                                           *
*       i      - utility variable.                                          *
*       ndx    - utility variable.                                          *
*                                                                           *
*       Arguments:                                                          *
*                                                                           *
*       lonray - array of longitude values to be simplified.                *
*       latray - array of latitude values to be simplified.                 *
*       count  - number of values in the arrays.                            *
*       gap    - increment for simplification.                              *
*                                                                           *
</pre>
****************************************************************************/

void simple (NV_FLOAT64 lonray[], NV_FLOAT64 latray[], NV_INT32 *count, NV_INT32 gap)
{
    NV_INT32         i, ndx;

    /*  If the increment is not 1 and the number of points in   */
    /*  the arrays is greater than 10, simplify the arrays.     */
    
    if (gap > 1 && *count > 10)
    {
        for (i = ndx = 0; i <= *count; i += gap, ndx++)
        {
            latray[ndx] = latray[i];
            lonray[ndx] = lonray[i];
        }
 
        latray[ndx] = latray[*count];
        lonray[ndx] = lonray[*count];

        /*  Set the count to the new count.                     */
        
        *count = ndx;
    }
}
