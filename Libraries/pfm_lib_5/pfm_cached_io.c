
/*********************************************************************************************

    This is public domain software that was developed by IVS under a Cooperative Research
    And Development Agreement (CRADA) with the Naval Oceanographic Office (NAVOCEANO) and
    was contributed to the Pure File Magic (PFM) Application Programming Interface (API)
    library.  It was later modified by SAIC under a separate CRADA with NAVOCEANO.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



/***************************************************************************\

    Even though this has my name on most of it, the majority of this code
    was done by the people at IVS.  I don't know exactly who did what as I
    got the code from Jeff Parker at SAIC but I'm pretty sure Will Burrow, 
    Danny Neville, Graeme Sweet had a lot to do with it.  This is some
    seriously cool stuff!

    Jan Depner, 06/21/07

\***************************************************************************/

/* Comment out the define to enable the assert statement in destroy_bin_cache(). */
/*
#define NDEBUG
#include <assert.h>
#include <inttypes.h>
*/

#define DESTROY_BIN_CACHE 1
#define MEM_DEBUG         0

static NV_INT32                 cache_max_rows = 4000, cache_max_cols = 4000;
static NV_INT32                 max_offset_rows = 2000, max_offset_cols = 2000;
static NV_INT32                 new_cache_max_rows = 2000, new_cache_max_cols = 2000;
static NV_INT32                 new_center_row = -1, new_center_col = -1;
static NV_INT32                 pfm_cache_size_max = 400000000;
static NV_INT32                 use_cov_flag = 0;

static NV_BOOL                  bin_cache_empty[MAX_PFM_FILES];
static BIN_RECORD_SUMMARY    ***bin_cache_rows[MAX_PFM_FILES];
static NV_INT32                 pfm_cache_size[MAX_PFM_FILES];
static NV_INT32                 pfm_cache_size_peak[MAX_PFM_FILES];
static NV_INT32                 offset_rows[MAX_PFM_FILES], offset_cols[MAX_PFM_FILES];

static NV_INT32                 pfm_cache_hit, pfm_cache_miss;
static NV_INT32                 pfm_cache_flushes = 0;
static NV_INT32                 force_cache_reset = 0;

/***************************************************************************/
/*!

  - Module Name:        open_cached_pfm_file

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Opens all of the associated files.

  - Restrictions:       This function will try to open and append to an
                        existing PFM list file.  If the specified list file
                        does not exist a new PFM structure will be created.
                        In this case all of the arguments in the
                        PFM_OPEN_ARGS structure need to be set as well as
                        the following members of the PFM_OPEN_ARGS.head
                        structure :
                            - bin_size_xy or (x_bin_size_degrees and
                              y_bin_size_degrees while setting bin_size_xy
                              to 0.0)
                            - polygon
                            - polygon_count
                            - proj_data.projection (and other proj data)
                            - num_bin_attr
                            - bin_attr_name[0 to num_bin_attr - 1]
                            - bin_attr_offset[0 to num_bin_attr - 1]
                            - bin_attr_max[0 to num_bin_attr - 1]
                            - bin_attr_scale[0 to num_bin_attr - 1]
                            - num_ndx_attr
                            - ndx_attr_name[0 to num_ndx_attr - 1]
                            - min_ndx_attr[0 to num_ndx_attr - 1]
                            - max_ndx_attr[0 to num_ndx_attr - 1]
                            - ndx_attr_scale[0 to num_ndx_attr - 1]
                            - user_flag_name[5]
                            - horizontal_error_scale
                            - max_horizontal_error
                            - vertical_error_scale
                            - max_vertical_error
                            - max_input_files (set to 0 for default)
                            - max_input_lines (set to 0 for default)
                            - max_input_pings (set to 0 for default)
                            - max_input_beams (set to 0 for default)

                        If bin_size_xy is 0.0 then we will use the
                        bin_sizes in degrees to define the final bin size
                        and the area dimensions.  This is useful for
                        defining PFMs for areas where you want to use
                        matching latitudinal and longitudinal bin sizes.
                        Pay special attention to checkpoint.  This can be
                        used to save/recover your file on an aborted load.
                        See pfm.h for better descriptions of these values.
                        <br><br>
                        IMPORTANT NOTE : For post 4.7 PFM structures the
                        file name in open_args->list_file will actually be
                        the PFM handle file name.  The actual PFM data
                        directory and structure file names will be derived
                        from the handle file name.  If the handle file name
                        is fred.pfm then a directory called fred.pfm.data
                        will exist (or be created) that contains the files
                        fred.pfm.ctl, fred.pfm.bin, fred.pfm.ndx,
                        fred.pfm.lin, and the rest of the PFM structure
                        files.  Pre 5.0 structures will use the actual list
                        file name here.

  - Arguments:          open_args       -   check pfm.h

  - Return Value:
                        - PFM file handle or -1 on error (error status stored
                          in pfm_error, can be printed with pfm_error_exit)
                        - Possible error status :
                            - OPEN_HANDLE_FILE_CREATE_ERROR
                            - CREATE_PFM_DATA_DIRECTORY_ERROR
                            - CREATE_LIST_FILE_FILE_EXISTS
                            - CREATE_LIST_FILE_OPEN_ERROR
                            - OPEN_LIST_FILE_OPEN_ERROR
                            - OPEN_LIST_FILE_READ_VERSION_ERROR
                            - OPEN_LIST_FILE_READ_BIN_ERROR
                            - OPEN_LIST_FILE_READ_INDEX_ERROR
                            - OPEN_LIST_FILE_READ_IMAGE_ERROR
                            - OPEN_LIST_FILE_CORRUPTED_FILE_ERROR
                            - OPEN_BIN_OPEN_ERROR
                            - OPEN_BIN_HEADER_CORRUPT_ERROR
                            - WRITE_BIN_HEADER_EXCEEDED_MAX_POLY
                            - OPEN_INDEX_OPEN_ERROR
                            - CHECK_INPUT_FILE_OPEN_ERROR
                            - CHECK_INPUT_FILE_WRITE_ERROR
                            - CHECKPOINT_FILE_EXISTS_ERROR
                            - CHECKPOINT_FILE_UNRECOVERABLE_ERROR

  - Caveats:                This code was actually developed by IVS in 
                            2007.  It was later modified by SAIC.  The
                            suspected culprits are listed below ;-)
                                - Danny Neville (IVS)
                                - Graeme Sweet (IVS)
                                - William Burrow (IVS)
                                - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 open_cached_pfm_file (PFM_OPEN_ARGS *open_args)
{
    NV_INT32   hnd;

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif


    hnd = open_pfm_file( open_args );

    if( hnd != -1 )
    {
        bin_cache_rows[hnd] = NULL;
        pfm_cache_size[hnd] = 0;
        pfm_cache_size_peak[hnd] = 0;
        bin_cache_empty[hnd] = NVTrue;

        pfm_cache_hit = 0;
        pfm_cache_miss = 0;
    }

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    return hnd;
}

/***************************************************************************/
/*!

  - Module Name:        close_pfm_file

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Closes all associated files.  Also, flushes
                        buffers.

  - Arguments:
                        - hnd           =   PFM file handle

  - Return Value:
                        - void

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

void close_cached_pfm_file (NV_INT32 hnd)
{

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    flush_bin_cache(hnd);
    destroy_bin_cache(hnd);

    close_pfm_file( hnd );

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

}

/***************************************************************************/
/*!

  - Module Name:        read_cached_bin_record

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Reads a bin record from the bin/index file.  This
                        function is only used internal to the library.

  - Arguments:
                        - hnd             =   PFM file handle
                        - bin             =   BIN_RECORD structure
                        - address         =   Address of record

  - Return Value:
                        - SUCCESS
                        - READ_BIN_RECORD_DATA_READ_ERROR

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 read_cached_bin_record (NV_INT32 hnd, NV_I32_COORD2 coord, BIN_RECORD_SUMMARY **bin_summary)
{
    NV_INT64           address;
    static NV_INT32    cacheRow, cacheCol, i;
    BIN_RECORD         tempBin;
    NV_U_BYTE          cov = 0;

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif


    /* Set up the cache index pointers. */
    if (bin_cache_rows[hnd] == NULL )
    {
        force_cache_reset = 0;
        
        if ((new_center_row < 0) || (new_center_col < 0))
          {
            new_center_row = coord.y;
            new_center_col = coord.x;
          }
#if 0
        else
          {
            printf ("Setting Cache Center to RC %d, %d ...\n", new_center_row, new_center_col);
          }
#endif
        if ((new_cache_max_rows != cache_max_rows) || (new_cache_max_cols != cache_max_cols))
          {
            cache_max_rows  = new_cache_max_rows;
            cache_max_cols  = new_cache_max_cols;
            max_offset_rows = (NV_INT32)(cache_max_rows * 0.5);
            max_offset_cols = (NV_INT32)(cache_max_cols * 0.5);
          }

        bin_cache_rows[hnd] = malloc( cache_max_rows * sizeof( BIN_RECORD_SUMMARY** ) );
        for( i = 0; i < cache_max_rows; i++ )
        {
            bin_cache_rows[hnd][i] = NULL;
        }
        bin_cache_empty[hnd] = NVTrue;

        pfm_cache_size[hnd] += cache_max_rows * sizeof( BIN_RECORD_SUMMARY** );
#if MEM_DEBUG
        printf ("ReadCacheBinRec:  Rows =    %x CacheSize = %d\n", bin_cache_rows[hnd], pfm_cache_size[hnd]);
        fflush(stdout);
#endif
        if( pfm_cache_size[hnd] > pfm_cache_size_peak[hnd] )
            pfm_cache_size_peak[hnd] = pfm_cache_size[hnd];
    }

    if( bin_cache_empty[hnd] )
      {
        new_center_row = coord.y;
        new_center_col = coord.x;

        offset_rows[hnd] = (new_center_row - cache_max_rows) + max_offset_rows + 1;
        if (offset_rows[hnd] < 0)
        {
            offset_rows[hnd] = 0;
        }

        offset_cols[hnd] = (new_center_col - cache_max_cols) + max_offset_cols + 1;
        if (offset_cols[hnd] < 0)
        {
            offset_cols[hnd] = 0;
        }

        bin_cache_empty[hnd] = NVFalse;
    }

    cacheRow = coord.y - offset_rows[hnd];
    cacheCol = coord.x - offset_cols[hnd];

    /*************************************************
     * Test for access outside the bounds of the cache.
     * CAUTION: recursive call - should only happen once though.
     */
    if( ( cacheRow < 0 ) || ( cacheRow >= cache_max_rows ) 
            || ( cacheCol < 0 ) || ( cacheCol >= cache_max_cols ) || force_cache_reset)
    {
#if 0
        printf ("Extent of Cache Exceeded:  Cache Size = %ld ", pfm_cache_size[hnd]);
        fflush(stdout);
#endif
        flush_bin_cache(hnd);
        
        if (DESTROY_BIN_CACHE)
          destroy_bin_cache(hnd);
#if 0
        printf (" => %ld \n", pfm_cache_size[hnd]);
        fflush(stdout);
#endif

        force_cache_reset = 0;

        return read_cached_bin_record (hnd, coord, bin_summary);
    }
    if ( pfm_cache_size[hnd] > pfm_cache_size_max )
    {
        printf ("MAX Cache Size (%d) Reached, Resetting from %d ", pfm_cache_size_max, pfm_cache_size[hnd]);

        flush_bin_cache(hnd);
        destroy_bin_cache(hnd);

        printf ("=> %d\n", pfm_cache_size[hnd]);
        fflush(stdout);

        return read_cached_bin_record (hnd, coord, bin_summary);
    }
    /*************************************************/

    if (bin_cache_rows[hnd][cacheRow] == NULL )
    {
        bin_cache_rows[hnd][cacheRow] = malloc( cache_max_cols * sizeof( BIN_RECORD_SUMMARY* ) );
        for( i = 0; i < cache_max_cols; i++ )
        {
            bin_cache_rows[hnd][cacheRow][i] = NULL;
        }
        pfm_cache_size[hnd] += cache_max_cols * sizeof( BIN_RECORD_SUMMARY* );
#if MEM_DEBUG
        printf ("ReadCacheBinRec:  Row = %d Cols =           %x CacheSize = %d\n", cacheRow, bin_cache_rows[hnd][cacheRow], pfm_cache_size[hnd]);
        fflush(stdout);
#endif
        if( pfm_cache_size[hnd] > pfm_cache_size_peak[hnd] )
            pfm_cache_size_peak[hnd] = pfm_cache_size[hnd];
    }

    if( bin_cache_rows[hnd][cacheRow][cacheCol] == NULL  || !bin_cache_rows[hnd][cacheRow][cacheCol]->dirty )
        pfm_cache_miss++;
    else
        pfm_cache_hit++;

    if (bin_cache_rows[hnd][cacheRow][cacheCol] == NULL )
    {
        /*  Allocate a new bin.  */

        if ((bin_cache_rows[hnd][cacheRow][cacheCol] = malloc (sizeof( BIN_RECORD_SUMMARY ))) == NULL)
        {
            sprintf (pfm_err_str, "Unable to allocate memory for bin record");
            return (pfm_error = SET_OFFSETS_BIN_MALLOC_ERROR);
        }

        pfm_cache_size[hnd] += sizeof( BIN_RECORD_SUMMARY );
#if MEM_DEBUG
        printf ("ReadCacheBinRec:  RC = %d, %d Summary =    %x CacheSize = %d\n", cacheRow, cacheCol, bin_cache_rows[hnd][cacheRow][cacheCol], pfm_cache_size[hnd]);
        fflush(stdout);
#endif
        memset( bin_cache_rows[hnd][cacheRow][cacheCol], 0, sizeof( BIN_RECORD_SUMMARY ));
    }

    if ( ! bin_cache_rows[hnd][cacheRow][cacheCol]->dirty )
    {
        /*  The bin has no data in it yet, fetch some from disk!  */

        bin_cache_rows[hnd][cacheRow][cacheCol]->dirty = NVTrue;

        address = ((NV_INT64) coord.y * (NV_INT64) bin_header[hnd].bin_width + (NV_INT64)coord.x) *
            (NV_INT64) bin_off[hnd].record_size + BIN_HEADER_SIZE;

        hfseek (bin_handle[hnd], address, SEEK_SET);

        /*  Read the record.  */
        if (!(hfread (bin_record_data[hnd], bin_off[hnd].record_size, 1, bin_handle[hnd])))
        {
            sprintf (pfm_err_str, "Error reading from bin file");
            return (pfm_error = READ_BIN_RECORD_DATA_READ_ERROR);
        }

        unpack_bin_record (hnd, bin_record_data[hnd], &tempBin ); 

        *bin_summary = bin_cache_rows[hnd][cacheRow][cacheCol];

        if (use_cov_flag)
          read_cov_map_index (hnd, coord, &cov);
        else
            cov = 0;

        (*bin_summary)->num_soundings = tempBin.num_soundings;
        (*bin_summary)->validity = tempBin.validity;
        (*bin_summary)->cov_flag = cov;
        (*bin_summary)->coord = coord;
        (*bin_summary)->depth.continuation_pointer = 0;
        (*bin_summary)->depth.record_pos = 0;
        (*bin_summary)->depth.buffers.depths = NULL;
        (*bin_summary)->depth.buffers.next = NULL;
        (*bin_summary)->depth.last_depth_buffer = NULL;
        (*bin_summary)->depth.previous_chain = 0;
        (*bin_summary)->depth.chain = tempBin.depth_chain;

    }
    else
    {
        *bin_summary = bin_cache_rows[hnd][cacheRow][cacheCol];
    }

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    return (pfm_error = SUCCESS);
}


/***************************************************************************/
/*!

  - Module Name:        write_cached_bin_record

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Writes a bin record to the bin/index file.  This
                        function is only used internal to the library.

  - Arguments:
                        - hnd             =   PFM file handle
                        - bin             =   BIN_RECORD structure
                        - address         =   Address of the record

  - Return Value:
                        - SUCCESS
                        - WRITE_BIN_RECORD_DATA_READ_ERROR

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 write_cached_bin_record (NV_INT32 hnd, BIN_RECORD_SUMMARY *bin_summary )
{
    NV_INT32            i, temp;
    static BIN_RECORD   *tmp_bin = NULL;

    /*  Transfer the bin summary to an actual bin record for writing to disk.  */

    if( tmp_bin == NULL )
    {
        tmp_bin = malloc(sizeof(BIN_RECORD));

        tmp_bin->standard_dev = 0;
        tmp_bin->avg_filtered_depth = 0;
        tmp_bin->min_filtered_depth = 0;
        tmp_bin->max_filtered_depth = 0;
        tmp_bin->avg_depth = 0;
        tmp_bin->min_depth = 0;
        tmp_bin->max_depth = 0;
        for( i = 0; i < NUM_ATTR; i++ )
        {
            tmp_bin->attr[i] = 0;
        }
    }

    tmp_bin->num_soundings = bin_summary->num_soundings;
    tmp_bin->validity      = bin_summary->validity;
    tmp_bin->coord         = bin_summary->coord;
    tmp_bin->depth_chain   = bin_summary->depth.chain;

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif


    /*  We can use the chain pointers passed to the caller instead of having
        to reread the bin record as long as we haven't done an
        add_depth_record since the file was opened.  This, of course, assumes
        that the caller hasn't messed with the chain pointers.
        DON'T DO THAT!!!!!!!!!!  'NUFF SAID?  */

    bin_record_head_pointer[hnd] = bin_summary->depth.chain.head;
    bin_record_tail_pointer[hnd] = bin_summary->depth.chain.tail;

    pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].num_soundings_pos, hd[hnd].count_bits, tmp_bin->num_soundings);

    pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].std_pos, hd[hnd].std_bits, NINT (tmp_bin->standard_dev * hd[hnd].std_scale));

    pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].avg_filtered_depth_pos, hd[hnd].depth_bits,
                  NINT ((tmp_bin->avg_filtered_depth + hd[hnd].depth_offset) * hd[hnd].depth_scale));

    pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].min_filtered_depth_pos, hd[hnd].depth_bits,
                  NINT ((tmp_bin->min_filtered_depth + hd[hnd].depth_offset) * hd[hnd].depth_scale));

    pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].max_filtered_depth_pos, hd[hnd].depth_bits,
                  NINT ((tmp_bin->max_filtered_depth + hd[hnd].depth_offset) * hd[hnd].depth_scale));

    pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].avg_depth_pos, hd[hnd].depth_bits,
                  NINT ((tmp_bin->avg_depth + hd[hnd].depth_offset) * hd[hnd].depth_scale));

    pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].min_depth_pos, hd[hnd].depth_bits,
                  NINT ((tmp_bin->min_depth + hd[hnd].depth_offset) * hd[hnd].depth_scale));

    pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].max_depth_pos, hd[hnd].depth_bits,
                  NINT ((tmp_bin->max_depth + hd[hnd].depth_offset) * hd[hnd].depth_scale));


    /*  Pre 4.0 version dependency.  */

    if (list_file_ver[hnd] < 40)
    {
        if (tmp_bin->validity & PFM_MODIFIED)
        {
            temp = 1;
        }
        else
        {
            temp = 0;
        }

        pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].edited_flag_pos, hd[hnd].edited_flag_bits, temp);

        if (tmp_bin->validity & PFM_CHECKED)
        {
            temp = 1;
        }
        else
        {
            temp = 0;
        }

        pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].checked_flag_pos, hd[hnd].checked_flag_bits, temp);

        if (tmp_bin->validity & PFM_SUSPECT)
        {
            temp = 1;
        }
        else
        {
            temp = 0;
        }

        pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].suspect_flag_pos, hd[hnd].suspect_flag_bits, temp);

        if (tmp_bin->validity & PFM_DATA)
        {
            temp = 1;
        }
        else
        {
            temp = 0;
        }

        pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].data_flag_pos, hd[hnd].data_flag_bits, temp);

        if (tmp_bin->validity & PFM_SELECTED_SOUNDING)
        {
            temp = 1;
        }
        else
        {
            temp = 0;
        }

        pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].selected_flag_pos, hd[hnd].selected_flag_bits, temp);

        if (tmp_bin->validity & PFM_CLASS_1)
        {
            temp = 1;
        }
        else
        {
            temp = 0;
        }

        pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].class1_flag_pos, hd[hnd].class1_flag_bits, temp);

        if (tmp_bin->validity & PFM_CLASS_2)
        {
            temp = 1;
        }
        else
        {
            temp = 0;
        }

        pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].class2_flag_pos, hd[hnd].class2_flag_bits, temp);


        pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].projected_x_pos, hd[hnd].projected_x_bits, 0);
    }
    else
    {
        for (i = 0 ; i < hd[hnd].head.num_bin_attr ; i++)
          {
            pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].attr_pos[i], hd[hnd].bin_attr_bits[i],
                          NINT ((tmp_bin->attr[i] + hd[hnd].bin_attr_offset[i]) * hd[hnd].head.bin_attr_scale[i]));
          }

        pfm_bit_pack (bin_record_data[hnd], bin_off[hnd].validity_pos, hd[hnd].validity_bits, tmp_bin->validity);
    }


    PFM_DBL_BIT_PACK (bin_record_data[hnd], bin_off[hnd].head_pointer_pos, hd[hnd].record_pointer_bits,
                      bin_record_head_pointer[hnd] );

    PFM_DBL_BIT_PACK (bin_record_data[hnd], bin_off[hnd].tail_pointer_pos, hd[hnd].record_pointer_bits,
                      bin_record_tail_pointer[hnd] );



    bin_record_address[hnd] = ((NV_INT64) bin_summary->coord.y * (NV_INT64) bin_header[hnd].bin_width + bin_summary->coord.x) *
        (NV_INT64) bin_off[hnd].record_size + BIN_HEADER_SIZE;

    write_bin_buffer_only (hnd, bin_record_address[hnd]);
    bin_record_modified[hnd] = NVFalse;

    /*  Reset the buffer and mark it as clean (bin->dirty == 0).  */

    memset( bin_summary, 0, sizeof( BIN_RECORD_SUMMARY ));



#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif


    return (pfm_error = SUCCESS);
}

/***************************************************************************/
/*!

  - Module Name:        write_cached_depth_buffer

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Writes a single 'physical' depth buffer to the
                        index file.  This function is only used internal to
                        the library.

  - Arguments:
                        - hnd             =   PFM file handle
                        - address         =   Address of record

  - Return Value:
                        - SUCCESS
                        - WRITE_DEPTH_BUFFER_WRITE_ERROR

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 write_cached_depth_buffer (NV_INT32 hnd, NV_U_BYTE *buffer,
        NV_INT64 address)
{
#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif


    hfseek (index_handle[hnd], address, SEEK_SET);



    if (!(hfwrite (buffer, dep_off[hnd].record_size, 1,
                    index_handle[hnd])))
    {
        sprintf (pfm_err_str, "Unable to write to index file");
        return (pfm_error = WRITE_DEPTH_BUFFER_WRITE_ERROR);
    }


#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif


    return (pfm_error = SUCCESS);
}

/***************************************************************************/
/*!

  - Module Name:        prepare_cached_depth_buffer

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Writes a single 'physical' depth buffer to the
                        index file.  This function is only used internal to
                        the library.

  - Arguments:
                        - hnd             =   PFM file handle
                        - address         =   Address of record

  - Return Value:
                        - SUCCESS
                        - WRITE_DEPTH_BUFFER_WRITE_ERROR

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

void prepare_cached_depth_buffer (NV_INT32 hnd, DEPTH_SUMMARY *depth, NV_U_BYTE *depth_buffer,
                                  NV_BOOL preallocateBuffer)
{

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    if (depth->continuation_pointer == 0)
    {

        /*  There is no known place reserved in the file to write the depth buffer, yet.  */

        if ( depth->chain.tail == -1 )
        {

            /*  This is the first time a depth buffer will be written for this bin.  */

            /*  This buffer and space for any associated buffer will be stored
             *  at the end of the file.  */

            hfseek (index_handle[hnd], 0, SEEK_END);

            /*  Set the head & tail pointers for the bin record.  */

            depth->chain.head = hftell (index_handle[hnd]);

            depth->chain.tail = depth->chain.head;


            if( preallocateBuffer )
            {
                depth->continuation_pointer = depth->chain.head + dep_off[hnd].record_size;
            }
        }
        else
        {
            /*  The record was swapped out of cache before a continuation record could be
             *  written.  This situation is not handled here. */

            fprintf( stderr, "Programming or hardware error in PFM cache code.\n");

        }
    }
    else
    {

        /*  There exists a place to write the buffer on the disk.  */

        depth->chain.tail = depth->continuation_pointer;

        if( preallocateBuffer )
        {
            depth->continuation_pointer = depth->chain.tail + dep_off[hnd].record_size;
        }
        else
        {
            depth->continuation_pointer = 0;
        }
    }

    /*  Store the updated continuation pointer.  */

    PFM_DBL_BIT_PACK (depth_buffer, dep_off[hnd].continuation_pointer_pos,
                      hd[hnd].record_pointer_bits, depth->continuation_pointer);

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

}


/***************************************************************************/
/*!

  - Module Name:        write_cached_depth_summary

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Similar to the above function, but prepares the
                        physical records on disk, if necessary.

  - Arguments:
                        - hnd             =   PFM file handle
                        - address         =   Address of record

  - Return Value:
                        - SUCCESS
                        - WRITE_DEPTH_BUFFER_WRITE_ERROR

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 write_cached_depth_summary (NV_INT32 hnd, BIN_RECORD_SUMMARY *bin_summary)
{
    NV_INT32 status;
    DEPTH_LIST *buffer;

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif


    /*  Fetch the buffers from memory and write them to disk.  */

    /*  Beware the flow of execution here, buffer is updated conditionally!  */

    if (bin_summary->num_soundings > 0) {

        buffer = &(bin_summary->depth.buffers);

        if ( bin_summary->depth.previous_chain != -1 )
        {
        
            /*  Write the first buffer if it was read from disk.  It may need an
             *  updated continuation pointer.  */
        
            if( buffer->next != NULL )
            {
                hfseek (index_handle[hnd], 0, SEEK_END);
                bin_summary->depth.continuation_pointer = hftell( index_handle[hnd] );
        
                /*  Store the updated continuation pointer.  */
        
                PFM_DBL_BIT_PACK (buffer->depths, dep_off[hnd].continuation_pointer_pos,
                                  hd[hnd].record_pointer_bits, bin_summary->depth.continuation_pointer);
            }
            if (buffer->depths != NULL) {
              status = write_cached_depth_buffer (hnd, buffer->depths, bin_summary->depth.previous_chain);
              if ( status != 0 )
                return (status);
            }
#if 0
            else {
              printf ("No buffer->depths at %d, %d ...\n", bin_summary->coord.y, bin_summary->coord.x);
              fflush(stdout);
            }
#endif
            buffer = buffer->next;
        }
        
        if( buffer != NULL )
        {
            while( buffer->next != NULL )
            {
                /*  Write the depth records to disk.  Set the continuation pointer to follow the
                 *  current record, there will be another record to write soon.  */
        
                prepare_cached_depth_buffer (hnd, &(bin_summary->depth), buffer->depths, NVTrue);
        
                status = write_cached_depth_buffer (hnd, buffer->depths, bin_summary->depth.chain.tail);
                if ( status != 0 )
                    return (status);
        
                buffer = buffer->next;
            }

            /*  Write the final depth record without a continuation pointer.  */
        
            prepare_cached_depth_buffer (hnd, &(bin_summary->depth), buffer->depths, NVFalse);
            status = write_cached_depth_buffer (hnd, buffer->depths, bin_summary->depth.chain.tail);
            if ( status != 0 )
                return (status);
        }
    }

#ifdef PFM_DEBUG
        fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    return (pfm_error = SUCCESS);
}

/***************************************************************************/
/*!

  - Module Name:        add_cached_depth_record

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Adds a new depth record to the data chain in the
                        index file for a specific bin in the bin file.
                        NOTE: If the PFM_MODIFIED in the depth validity
                        bits is set, the EDITED FLAG bit is set in the bin
                        record associated with this depth.
                        NOTE: If the PFM_SELECTED_SOUNDING flag is set in
                        the depth validity bits, the SELECTED FLAG is set
                        in the bin record associated with this depth.
                        This function is only used internal to the library.

  - Arguments:
                        - hnd             =   PFM file handle
                        - depth           =   DEPTH_RECORD structure

  - Return Value:
                        - SUCCESS
                        - ADD_DEPTH_RECORD_READ_BIN_RECORD_ERROR
                        - ADD_DEPTH_RECORD_TOO_MANY_SOUNDINGS_ERROR
                        - WRITE_BIN_RECORD_DATA_READ_ERROR
                        - FILE_NUMBER_TOO_LARGE_ERROR
                        - LINE_NUMBER_TOO_LARGE_ERROR
                        - PING_NUMBER_TOO_LARGE_ERROR
                        - BEAM_NUMBER_TOO_LARGE_ERROR
                        - ADD_DEPTH_RECORD_OUT_OF_LIMITS_ERROR

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 add_cached_depth_record (NV_INT32 hnd, DEPTH_RECORD *depth)
{
    NV_INT32            status = SUCCESS;
    BIN_RECORD_SUMMARY  *bin_summary;

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif


    /* Safety check, to make sure the X/Y fits within the limits of the PFM */
    /* Webb McDonald -- Tue May 31 00:39:42 2005 */

    if (depth->xyz.x > bin_header[hnd].mbr.max_x || depth->xyz.x < bin_header[hnd].mbr.min_x ||
        depth->xyz.y > bin_header[hnd].mbr.max_y || depth->xyz.y < bin_header[hnd].mbr.min_y)
    {
        sprintf (pfm_err_str, "Error, trying to add_depth_record outside limits of PFM!\n");
        return (pfm_error = ADD_DEPTH_RECORD_OUT_OF_LIMITS_ERROR);
    }


    /*  Other safety checks, make sure that the ping, beam, file, and line numbers are in range.  */

    if (depth->file_number > hd[hnd].head.max_input_files)
      {
        sprintf (pfm_err_str, "Error, file number %d too large!\n", depth->file_number);
        return (pfm_error = FILE_NUMBER_TOO_LARGE_ERROR);
      }

    if (depth->line_number > hd[hnd].head.max_input_lines)
      {
        sprintf (pfm_err_str, "Error, line number %d too large!\n", depth->line_number);
        return (pfm_error = LINE_NUMBER_TOO_LARGE_ERROR);
      }

    if (depth->ping_number > hd[hnd].head.max_input_pings)
      {
        sprintf (pfm_err_str, "Error, record number %d too large!\n", depth->ping_number);
        return (pfm_error = PING_NUMBER_TOO_LARGE_ERROR);
      }

    if (depth->beam_number > hd[hnd].head.max_input_beams)
      {
        sprintf (pfm_err_str, "Error, subrecord number %d too large!\n", depth->beam_number);
        return (pfm_error = BEAM_NUMBER_TOO_LARGE_ERROR);
      }


#if MEM_DEBUG
            printf ("To AddCacheDepthRec:  RC = %2d, %2d XYZ = %0.2f, %0.2f, %6.2f  File = %2d Line = %2d Ping = %5d Beam = %3d\n", 
              depth->coord.y, depth->coord.x, depth->xyz.x, depth->xyz.y, depth->xyz.z, 
              depth->file_number, depth->line_number, depth->ping_number, depth->beam_number);
            fflush(stdout);
#endif
    if (read_cached_bin_record(hnd, depth->coord, &bin_summary))
    {
        sprintf (pfm_err_str, "Error reading bin record in add_depth_record");
        return (pfm_error = ADD_DEPTH_RECORD_READ_BIN_RECORD_ERROR);
    }



    if (bin_summary->num_soundings != 0)
    {
        /*  Compute the position within the buffer.  */

        bin_summary->depth.record_pos = (bin_summary->num_soundings % hd[hnd].record_length) * dep_off[hnd].single_point_bits;


        /*
           BUT DO read if depth_record_data is uninitialized...
           This prevents a corruption issue that can occur if you are
           appending data to the PFM that is *new* and you have had no
           reason to read the depth_record... UNTIL NOW, where you must.

           Webb McDonald -- Thu May 26 19:07:43 2005
        */
        if( bin_summary->depth.last_depth_buffer == 0x0 ) 
        {
            /* There is no data in the depth buffer, but there should be,
             * so load it from disk. */

            bin_summary->depth.buffers.depths = malloc( dep_off[hnd].record_size );
            bin_summary->depth.buffers.next = NULL;

            pfm_cache_size[hnd] += dep_off[hnd].record_size;
#if MEM_DEBUG
            printf ("0-AddCacheDepthRec:  buffers.depths =      %x CacheSize = %d\n", bin_summary->depth.buffers.depths, pfm_cache_size[hnd]);
            fflush(stdout);
#endif
            if( pfm_cache_size[hnd] > pfm_cache_size_peak[hnd] )
                pfm_cache_size_peak[hnd] = pfm_cache_size[hnd];


            bin_summary->depth.last_depth_buffer = &(bin_summary->depth.buffers);

            /*  Move to the tail of the data chain.  */

            hfseek (index_handle[hnd], bin_summary->depth.chain.tail, SEEK_SET);

            bin_summary->depth.previous_chain = bin_summary->depth.chain.tail;


            /*  Read the 'physical' depth record.  */

            hfread (bin_summary->depth.buffers.depths, dep_off[hnd].record_size, 1, index_handle[hnd]);

            /* Need to retrieve continuation pointer from depth buffer.  */
            /* XXX Should be zero!  */

            bin_summary->depth.continuation_pointer = PFM_DBL_BIT_UNPACK ( bin_summary->depth.buffers.depths,
                                                                           dep_off[hnd].continuation_pointer_pos,
                                                                           hd[hnd].record_pointer_bits);
            if( bin_summary->depth.continuation_pointer != 0 )
            {
                /*  Break point for debugging.  */
                fprintf( stderr, "Continuation pointer not zero on read: "NV_INT64_SPECIFIER"\n", bin_summary->depth.continuation_pointer );

                bin_summary->depth.continuation_pointer = 0;
            }

        }

        if( bin_summary->depth.record_pos == 0 )
        {

            /*  Buffer is full.  */
            /*  Save the record for writing later. */

            /*  Setup depth buffer linked list.  */

            bin_summary->depth.last_depth_buffer->next = malloc(sizeof( DEPTH_LIST ));
            bin_summary->depth.last_depth_buffer = bin_summary->depth.last_depth_buffer->next;

            pfm_cache_size[hnd] += sizeof( DEPTH_LIST );
#if MEM_DEBUG
            printf ("1-AddCacheDepthRec:  buffers->next =       %x CacheSize = %d\n", bin_summary->depth.last_depth_buffer, pfm_cache_size[hnd]);
            fflush(stdout);
#endif
            /*  Prep a new record.  */

            bin_summary->depth.last_depth_buffer->depths = malloc( dep_off[hnd].record_size );
            bin_summary->depth.last_depth_buffer->next = NULL;

            pfm_cache_size[hnd] += dep_off[hnd].record_size;
#if MEM_DEBUG
            printf ("2-AddCacheDepthRec:  buffers->depths =     %x CacheSize = %d\n", bin_summary->depth.last_depth_buffer->depths, pfm_cache_size[hnd]);
            fflush(stdout);
#endif
            if( pfm_cache_size[hnd] > pfm_cache_size_peak[hnd] )
                pfm_cache_size_peak[hnd] = pfm_cache_size[hnd];
        }
    }
    else
    {
        /*  Set the head and tail pointers to indicate that disk space has yet to be allocated
         *  for the depth record.  */

        bin_summary->depth.chain.head = -1;

        bin_summary->depth.chain.tail = bin_summary->depth.chain.head;

        bin_summary->depth.previous_chain = -1;


        /* Setup the depth buffer. */

        bin_summary->depth.buffers.depths = malloc( dep_off[hnd].record_size );
        bin_summary->depth.buffers.next = NULL;

        pfm_cache_size[hnd] += dep_off[hnd].record_size;
#if MEM_DEBUG
        printf ("3-AddCacheDepthRec:  buffers.depths =      %x CacheSize = %d\n", bin_summary->depth.buffers.depths, pfm_cache_size[hnd]);
        fflush(stdout);
#endif
        if( pfm_cache_size[hnd] > pfm_cache_size_peak[hnd] )
            pfm_cache_size_peak[hnd] = pfm_cache_size[hnd];


        bin_summary->depth.last_depth_buffer = &(bin_summary->depth.buffers);


        /*  Zero out the depth buffer.  */

        memset (bin_summary->depth.last_depth_buffer->depths, 0, dep_off[hnd].record_size);

        bin_summary->depth.record_pos = 0;

    }



    /*  Pack the current depth record into the 'physical' record.  */

    pack_depth_record( bin_summary->depth.last_depth_buffer->depths, depth, bin_summary->depth.record_pos, hnd );


    /*  Increment the number of soundings.  */

    bin_summary->num_soundings++;


    /*  Make sure that we don't blow out the number of soundings count.  */

    if (bin_summary->num_soundings >= count_size[hnd])
    {
        sprintf (pfm_err_str, "Too many soundings in this cell - %d %d.\n", bin_record[hnd].num_soundings,
                 count_size[hnd]);
        return (pfm_error = ADD_DEPTH_RECORD_TOO_MANY_SOUNDINGS_ERROR);
    }


    /*  Set the bin_record_address so that the write_bin_record function
        will know that this record has already been read and the head and
        tail pointers are known.  */


    /*  Set the bin validity to the same as the depth validity.  */

    bin_summary->validity |= depth->validity;


#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif


    return (pfm_error = status);
}

NV_INT32 recompute_cached_bin_values (NV_INT32 hnd, BIN_RECORD *bin, NV_U_INT32 mask, DEPTH_RECORD *depth)
{
    return recompute_bin_values(hnd, bin, mask, depth);
}


/***************************************************************************/
/*!

  - Module Name:        flush_depth_cache

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Closes the bin file.

  - Arguments:
                        - hnd         =   PFM file handle

  - Return Value:
                        - SUCCESS
                        - WRITE_BIN_BUFFER_WRITE_ERROR

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 flush_depth_cache (NV_INT32 hnd)
{
    NV_INT32  r, c;
    NV_INT32  status;


#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif


    if( bin_cache_rows[hnd] != NULL )
    {
        pfm_cache_flushes++;

        for( r = 0; r < cache_max_rows; r++ )
        {
            if( bin_cache_rows[hnd][r] == NULL )
                continue;

            for( c = 0; c < cache_max_cols; c++ )
            {
                if (bin_cache_rows[hnd][r][c] != NULL) 
                {
                  if (bin_cache_rows[hnd][r][c]->dirty)
                  {

                    /*  Write the depth record associated with the bin. */

                    if ((bin_cache_rows[hnd][r][c]->dirty == 1) || (bin_cache_rows[hnd][r][c]->dirty == 2)) {
                      status = write_cached_depth_summary (hnd, bin_cache_rows[hnd][r][c]);
                      if ( status != 0 )
                      {
                        destroy_depth_buffer( hnd, &(bin_cache_rows[hnd][r][c]->depth) );
                        return (status);
                      }
                    }

                    /*  Destroy the depth buffers.  */
                  }
#if MEM_DEBUG
                  printf ("To DestroyDepthBuffer: RC = %2d, %2d\n", r, c);
                  fflush(stdout);
#endif
                  destroy_depth_buffer( hnd, &(bin_cache_rows[hnd][r][c]->depth) );

                }
            }
        }
    }


#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    return(pfm_error = SUCCESS);
}

/***************************************************************************/
/*!

  - Module Name:        destroy_depth_buffer

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Frees a depth buffer.

  - Arguments:
                        - hnd         =   PFM file handle
                        - depth       =   The depth buffer to be freed.

  - Return Value:
                        - void

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

void destroy_depth_buffer ( NV_INT32 hnd, DEPTH_SUMMARY *depth)
{
    DEPTH_LIST  *buffer, *next;


#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    buffer = &(depth->buffers);

    if( buffer->depths != NULL )
    {
        pfm_cache_size[hnd] -= dep_off[hnd].record_size;
#if MEM_DEBUG
        printf ("0-DestroyDepthBuffer:  Buffer->depths = %x CacheSize = %d\n", buffer->depths, pfm_cache_size[hnd]);
        fflush(stdout);
#endif
        free( buffer->depths );
        buffer->depths = NULL;
    }

    buffer = buffer->next;
    while( buffer != NULL )
    {
        pfm_cache_size[hnd] -= dep_off[hnd].record_size;
#if MEM_DEBUG
        printf ("1-DestroyDepthBuffer:  Buffer->depths = %x CacheSize = %d\n", buffer->depths, pfm_cache_size[hnd]);
        fflush(stdout);
#endif
        free( buffer->depths );
        buffer->depths = NULL;

        next = buffer->next;

        pfm_cache_size[hnd] -= sizeof( *buffer );
#if MEM_DEBUG
        printf ("2-DestroyDepthBuffer:  Buffer         = %x CacheSize = %d\n", buffer, pfm_cache_size[hnd]);
        fflush(stdout);
#endif
        free( buffer );
        buffer = next;
    }
    depth->last_depth_buffer = NULL;


#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

}

/***************************************************************************/
/*!

  - Module Name:        flush_cov_flag

  - Programmer(s):      J. Parker

  - Date Written:       May 2007

  - Purpose:            Writes cov_flag to coverage map in bin file.

  - Arguments:
                        - hnd         =   PFM file handle

  - Return Value:
                        - SUCCESS
                        - WRITE_BIN_BUFFER_WRITE_ERROR

****************************************************************************/

NV_INT32 flush_cov_flag (NV_INT32 hnd)
{
    NV_INT32      r, c;
    NV_I32_COORD2 coord;

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    if( bin_cache_rows[hnd] != NULL )
    {
        for( r = 0; r < cache_max_rows; r++ )
        {
            if( bin_cache_rows[hnd][r] == NULL )
                continue;

            for( c = 0; c < cache_max_cols; c++ )
            {
                if( bin_cache_rows[hnd][r][c] != NULL )
                {
                  coord.x = c + offset_cols[hnd];
                  coord.y = r + offset_rows[hnd];

                  write_cov_map_index (hnd, coord, bin_cache_rows[hnd][r][c]->cov_flag);
                }
            }
        }
    }


#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    return(pfm_error = SUCCESS);
}

/***************************************************************************/
/*!

  - Module Name:        flush_bin_cache

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Closes the bin file.

  - Arguments:
                        - hnd         =   PFM file handle

  - Return Value:
                        - SUCCESS
                        - WRITE_BIN_BUFFER_WRITE_ERROR

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 flush_bin_cache (NV_INT32 hnd)
{
    NV_INT32  r, c;


#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

#ifdef PFM_DEBUG
    dump_cached_records (hnd);    /* for debugging purposes */
#endif

    /*  Flush the depths before the bins.  */

    flush_depth_cache (hnd);


    /*  Flush the coverage map flag (used by SAIC).  */

    if (use_cov_flag)
        flush_cov_flag (hnd);


    /*  Flush the bins.  */

    if( bin_cache_rows[hnd] != NULL )
    {
        pfm_cache_flushes++;

        for( r = 0; r < cache_max_rows; r++ )
        {
            if( bin_cache_rows[hnd][r] == NULL )
                continue;

            for( c = 0; c < cache_max_cols; c++ )
            {
                if(( bin_cache_rows[hnd][r][c] != NULL ) && 
                   (( bin_cache_rows[hnd][r][c]->dirty == 1 ) || ( bin_cache_rows[hnd][r][c]->dirty == 2 )))
                {

                    /*  Write the bin and associated depth record. */

                    if( write_cached_bin_record( hnd, bin_cache_rows[hnd][r][c] ) )
                    {
                        return pfm_error;
                    }

                    /*  Destroy the depth buffers.  */

/*                    destroy_depth_buffer( hnd, &(bin_cache_rows[hnd][r][c]->depth) ); */

                }
            }
        }
    }

    bin_cache_empty[hnd] = NVTrue;


#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    return(pfm_error = SUCCESS);
}

/***************************************************************************/
/*!

  - Module Name:        destroy_bin_cache

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1998

  - Purpose:            Frees the bin cache.  Should be called AFTER
                        flush_bin_cache.

  - Arguments:
                        - hnd         =   PFM file handle

  - Return Value:
                        - SUCCESS
                        - WRITE_BIN_BUFFER_WRITE_ERROR

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 destroy_bin_cache (NV_INT32 hnd)
{
    NV_INT32    r, c;

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif


    if( bin_cache_rows[hnd] != NULL )
    {
        for( r = 0; r < cache_max_rows; r++ )
        {
            if( bin_cache_rows[hnd][r] == NULL )
                continue;

            for( c = 0; c < cache_max_cols; c++ )
            {
                if( bin_cache_rows[hnd][r][c] != NULL )
                {
/*                  destroy_depth_buffer( hnd, &(bin_cache_rows[hnd][r][c]->depth) ); */

                    pfm_cache_size[hnd] -= sizeof( *(bin_cache_rows[hnd][r][c]) );

#if MEM_DEBUG
        printf ("0-DestroyBinCache (%d, %d):  BinCacheCols = %x CacheSize = %d\n", r, c, bin_cache_rows[hnd][r][c], pfm_cache_size[hnd]);
        fflush(stdout);
#endif
                    free( bin_cache_rows[hnd][r][c] );
                    bin_cache_rows[hnd][r][c] = NULL;
                }
            }

            pfm_cache_size[hnd] -= cache_max_cols * sizeof( *(bin_cache_rows[hnd][r]) );
#if MEM_DEBUG
        printf ("1-DestroyBinCache (%d):      BinCacheRows = %x CacheSize = %d\n", r, bin_cache_rows[hnd][r], pfm_cache_size[hnd]);
        fflush(stdout);
#endif
            free( bin_cache_rows[hnd][r] );
            bin_cache_rows[hnd][r] = NULL;
        }

        pfm_cache_size[hnd] -= cache_max_rows * sizeof( *(bin_cache_rows[hnd]) );
#if MEM_DEBUG
        printf ("2-DestroyBinCache:  BinCacheRows = %x CacheSize = %d\n", bin_cache_rows[hnd], pfm_cache_size[hnd]);
        fflush(stdout);
#endif
        free( bin_cache_rows[hnd] );
        bin_cache_rows[hnd] = NULL;
    }
  /*
    assert( pfm_cache_size[hnd] == 0 );
  */
#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    return(pfm_error = SUCCESS);
}


/***************************************************************************/
/*!

  - Module Name:        set_cached_cov_flag

  - Programmer(s):      J. Parker

  - Date Written:       May 2007

  - Purpose:            Sets coverage byte in cache.

  - Arguments:
                        - hnd             =   PFM file handle
                        - coord           =   COORD2 structure
                        - flag            =   Coverage flag

  - Return Value:
                        - SUCCESS

****************************************************************************/

NV_INT32 set_cached_cov_flag (NV_INT32 hnd, NV_I32_COORD2 coord, NV_U_BYTE flag)
{
    NV_INT32            status = SUCCESS;
    BIN_RECORD_SUMMARY  *bin_summary;

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    if (read_cached_bin_record(hnd, coord, &bin_summary))
    {
      sprintf (pfm_err_str, "Error reading bin record in set_cached_cov_flag");
      return (pfm_error = ADD_DEPTH_RECORD_READ_BIN_RECORD_ERROR);
    }

    bin_summary->cov_flag |= flag;

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    return (pfm_error = status);
}

void set_use_cov_flag (NV_INT32 flag)
{
    use_cov_flag = flag;
    printf ("Using Coverage Flag ...\n");
}

void set_cache_size (NV_INT32 max_rows, NV_INT32 max_cols, NV_INT32 row, NV_INT32 col)
{

    if ((max_rows > 0) && (max_cols > 0)) {
        /* insure that new values are even numbers */
        new_cache_max_rows = (NV_INT32)(floor(max_rows * 0.5) * 2.0); 
        new_cache_max_cols = (NV_INT32)(floor(max_cols * 0.5) * 2.0); 
        printf ("Setting Cache Size to %d rows X %d columns ...\n", new_cache_max_rows, new_cache_max_cols);
    }
    if ((row >= 0) && (col >= 0)) {
        new_center_row = row;
        new_center_col = col; 
    }
    force_cache_reset = 1;
}

void set_cache_size_max (NV_INT32 max)
{
    pfm_cache_size_max = max;
    printf ("Setting MAXIMUM Cache Size to %d ...\n", pfm_cache_size_max);
}

NV_INT32 get_cache_hits( void )
{
    return pfm_cache_hit;
}

NV_INT32 get_cache_misses( void )
{
    return pfm_cache_miss;
}

NV_INT32 get_cache_flushes( void )
{
    return pfm_cache_flushes;
}

NV_INT32 get_cache_size( NV_INT32 hnd )
{
    return pfm_cache_size[hnd];
}

NV_INT32 get_cache_peak_size( NV_INT32 hnd )
{
    return pfm_cache_size_peak[hnd];
}

NV_INT32 get_cache_max_size( NV_INT32 hnd )
{
    return pfm_cache_size_max;
}


/***************************************************************************/
/*!

  - Module Name:        read_cached_depth_records

  - Programmer(s):

  - Date Written:

  - Purpose:            Reads depth records from the index file.  This
                        function is only used internal to the library.

  - Arguments:
                        - hnd             =   PFM file handle
                        - bin_summary     =   BIN_RECORD_SUMMARY pointer

  - Return Value:       1

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 read_cached_depth_records (NV_INT32 hnd, BIN_RECORD_SUMMARY **bin_summary)
{
    DEPTH_LIST    *buffer;

#if MEM_DEBUG
        printf ("ReadCacheDepthRec:  Sndgs = %2d  RC = %2d, %2d\n", 
           (*bin_summary)->num_soundings, (*bin_summary)->coord.y, (*bin_summary)->coord.x);
        fflush(stdout);
#endif

    if ((*bin_summary)->num_soundings > 0)
    {
        buffer = &((*bin_summary)->depth.buffers);
        
        (*bin_summary)->depth.record_pos = ((*bin_summary)->num_soundings % hd[hnd].record_length) * dep_off[hnd].single_point_bits;
        
        if (buffer->depths != NULL)
          return 1;

        buffer->depths = malloc( dep_off[hnd].record_size );
        buffer->next = NULL;

        pfm_cache_size[hnd] += dep_off[hnd].record_size;
#if MEM_DEBUG
        printf ("ReadCacheDepthRec (%2d): 0-Buffer->depths = %x CacheSize = %d\n", 
          (*bin_summary)->num_soundings, buffer->depths, pfm_cache_size[hnd]);
        fflush(stdout);
#endif
        if( pfm_cache_size[hnd] > pfm_cache_size_peak[hnd] )
            pfm_cache_size_peak[hnd] = pfm_cache_size[hnd];

        (*bin_summary)->depth.last_depth_buffer = &((*bin_summary)->depth.buffers);
        
        /*  Move to the head of the data chain.  */

        hfseek (index_handle[hnd], (*bin_summary)->depth.chain.head, SEEK_SET);
        (*bin_summary)->depth.previous_chain = (*bin_summary)->depth.chain.head;

        /*  Read the 'physical' depth record.  */

        hfread (buffer->depths, dep_off[hnd].record_size, 1, index_handle[hnd]);
#if 0
       if ((*bin_summary)->num_soundings > 6) {
          printf ("\nRead - RC = %ld, %ld Num = %ld: \n", (*bin_summary)->coord.y, (*bin_summary)->coord.x, (*bin_summary)->num_soundings);
          for (k = 0; k < 131; k++) {
            if (!(k % 21) && (k > 0)) printf ("\n");
            printf (" %02x", buffer->depths[k]);
          }
          fflush(stdout);
        }
#endif
        /* Need to retrieve continuation pointer from depth buffer.  */

        (*bin_summary)->depth.continuation_pointer = PFM_DBL_BIT_UNPACK ( (*bin_summary)->depth.buffers.depths,
                                                                          dep_off[hnd].continuation_pointer_pos,
                                                                          hd[hnd].record_pointer_bits);
        
        while ((*bin_summary)->depth.continuation_pointer != 0)
        {
            hfseek (index_handle[hnd], (*bin_summary)->depth.continuation_pointer, SEEK_SET);
            
            buffer->next = (DEPTH_LIST *)malloc(sizeof(DEPTH_LIST));
            pfm_cache_size[hnd] += sizeof( DEPTH_LIST );
#if MEM_DEBUG
            printf ("ReadCacheDepthRec:  Buffer->next =         %x CacheSize = %d\n", buffer->next, pfm_cache_size[hnd]);
            fflush(stdout);
#endif
            buffer = buffer->next;
            
            buffer->next = NULL;

            buffer->depths = (NV_U_BYTE *)malloc( dep_off[hnd].record_size );
            pfm_cache_size[hnd] += dep_off[hnd].record_size;
#if MEM_DEBUG
            printf ("ReadCacheDepthRec:  1-Buffer->depths =     %x CacheSize = %d\n", buffer->depths, pfm_cache_size[hnd]);
            fflush(stdout);
#endif
            if( pfm_cache_size[hnd] > pfm_cache_size_peak[hnd] )
                pfm_cache_size_peak[hnd] = pfm_cache_size[hnd];

            hfread (buffer->depths, dep_off[hnd].record_size, 1, index_handle[hnd]);
#if 0            
            printf ("\nRead - RC = %ld, %ld Num = %ld: \n", (*bin_summary)->coord.y, (*bin_summary)->coord.x, (*bin_summary)->num_soundings);
            for (k = 0; k < 131; k++) {
              if (!(k % 21) && (k > 0)) printf ("\n");
              printf (" %02x", buffer->depths[k]);
            }
            fflush(stdout);
#endif
            (*bin_summary)->depth.continuation_pointer = PFM_DBL_BIT_UNPACK ( buffer->depths,
                                                                              dep_off[hnd].continuation_pointer_pos,
                                                                              hd[hnd].record_pointer_bits);
        }
    }
    return 1;
}

/***************************************************************************/
/*!

  - Module Name:        put_cached_depth_records

  - Programmer(s):

  - Date Written:

  - Purpose:

  - Arguments:

  - Return Value:       1

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 put_cached_depth_records (NV_INT32 hnd, NV_I32_COORD2 coord, DEPTH_RECORD **depth, BIN_RECORD_SUMMARY **bsum)
{
    NV_INT32            i = 0, j, record_pos;
    DEPTH_LIST         *buffer;
    NV_FLOAT32          x_offset, y_offset;

    record_pos = 0;

      buffer = &((*bsum)->depth.buffers);
      if ((*bsum)->num_soundings > 0) {
        (*bsum)->dirty = 2;
          while (buffer != NULL) {

            /*  pack the current depth record into the 'physical' record.  */
            pfm_bit_pack (buffer->depths, dep_off[hnd].file_number_pos + record_pos, hd[hnd].file_number_bits,
                          (*depth)[i].file_number);
            
            pfm_bit_pack (buffer->depths, dep_off[hnd].line_number_pos + record_pos, hd[hnd].line_number_bits,
                          (*depth)[i].line_number);
            
            pfm_bit_pack (buffer->depths, dep_off[hnd].ping_number_pos + record_pos, hd[hnd].ping_number_bits,
                          (*depth)[i].ping_number);
            
            pfm_bit_pack (buffer->depths, dep_off[hnd].beam_number_pos + record_pos, hd[hnd].beam_number_bits,
                          (*depth)[i].beam_number);
            
            pfm_bit_pack (buffer->depths, dep_off[hnd].depth_pos + record_pos, hd[hnd].depth_bits,
                          NINT (((*depth)[i].xyz.z + hd[hnd].depth_offset) * hd[hnd].depth_scale));
            
            /*  Compute the offsets from the position.   */
            /*  Stored as lat/lon.  */
            if (!hd[hnd].head.proj_data.projection)
            {
                x_offset = (*depth)[i].xyz.x - (bin_header[hnd].mbr.min_x + ((*depth)[i].coord.x * bin_header[hnd].x_bin_size_degrees));
                y_offset = (*depth)[i].xyz.y - (bin_header[hnd].mbr.min_y + ((*depth)[i].coord.y * bin_header[hnd].y_bin_size_degrees));
            
            
                pfm_bit_pack (buffer->depths, dep_off[hnd].x_offset_pos + record_pos, hd[hnd].offset_bits,
                              NINT ((x_offset / bin_header[hnd].x_bin_size_degrees) * x_offset_scale[hnd]));
            
                pfm_bit_pack (buffer->depths, dep_off[hnd].y_offset_pos + record_pos, hd[hnd].offset_bits,
                              NINT ((y_offset / bin_header[hnd].y_bin_size_degrees) * y_offset_scale[hnd]));
            }
            
            /*  Stored as x/y.  */
            else
            {
                x_offset = (*depth)[i].xyz.x - (bin_header[hnd].mbr.min_x + ((*depth)[i].coord.x * bin_header[hnd].bin_size_xy));
                y_offset = (*depth)[i].xyz.y - (bin_header[hnd].mbr.min_y + ((*depth)[i].coord.y * bin_header[hnd].bin_size_xy));
            
            
                pfm_bit_pack (buffer->depths, dep_off[hnd].x_offset_pos + record_pos, hd[hnd].offset_bits,
                              NINT ((x_offset / bin_header[hnd].bin_size_xy) * x_offset_scale[hnd]));
            
                pfm_bit_pack (buffer->depths, dep_off[hnd].y_offset_pos + record_pos, hd[hnd].offset_bits,
                              NINT ((y_offset / bin_header[hnd].bin_size_xy) * y_offset_scale[hnd]));
            }
            
            /*  Pre 4.0 version dependency.  */
            if (list_file_ver[hnd] < 40)
            {
                pfm_bit_pack (buffer->depths, dep_off[hnd].validity_pos + record_pos, hd[hnd].validity_bits, (*depth)[i].validity);
            }
            else
            {
                for (j = 0 ; j < hd[hnd].head.num_ndx_attr ; j++)
                {
                    pfm_bit_pack (buffer->depths, dep_off[hnd].attr_pos[j] + record_pos, hd[hnd].ndx_attr_bits[j],
                                  NINT (((*depth)[i].attr[j] - hd[hnd].head.min_ndx_attr[j]) * hd[hnd].head.ndx_attr_scale[j]));
                }
            
                pfm_bit_pack (buffer->depths, dep_off[hnd].validity_pos + record_pos, hd[hnd].validity_bits, (*depth)[i].validity);
            }
            if (hd[hnd].horizontal_error_bits)
            {
                if ((*depth)[i].horizontal_error >= hd[hnd].horizontal_error_null)
                    (*depth)[i].horizontal_error = hd[hnd].horizontal_error_null;
                pfm_bit_pack (buffer->depths, dep_off[hnd].horizontal_error_pos + record_pos,
                              hd[hnd].horizontal_error_bits, NINT ((*depth)[i].horizontal_error * hd[hnd].head.horizontal_error_scale));
            }
            if (hd[hnd].vertical_error_bits)
            {
                if ((*depth)[i].vertical_error >= hd[hnd].vertical_error_null) (*depth)[i].vertical_error = hd[hnd].vertical_error_null;
                pfm_bit_pack (buffer->depths, dep_off[hnd].vertical_error_pos + record_pos,
                              hd[hnd].vertical_error_bits, NINT ((*depth)[i].vertical_error * hd[hnd].head.vertical_error_scale));
            }
                            
            record_pos += dep_off[hnd].single_point_bits;
            i++;

            if (!(i % hd[hnd].record_length)) {
#if 0
              if (bsum->num_soundings > hd[hnd].record_length) {
                printf ("\nPUT - RC = %ld, %ld I = %d Num = %ld: \n", coord.y, coord.x, i, bsum->num_soundings);
                for (k = 0; k < 131; k++) {
                  if (!(k % 21) && (k > 0)) printf ("\n");
                  printf (" %02x", buffer->depths[k]);
                }
                fflush(stdout);
              }
#endif
            }
            if (i >= (*bsum)->num_soundings) {
#if 0
                printf ("\nPUT - RC = %ld, %ld I = %d Num = %ld: \n", coord.y, coord.x, i, bsum->num_soundings);
                for (k = 0; k < 131; k++) {
                  if (!(k % 21) && (k > 0)) printf ("\n");
                  printf (" %02x", buffer->depths[k]);
                }
                fflush(stdout);
#endif
              break;
            }
            if ((i > 0) && !(i % hd[hnd].record_length)) {
              record_pos = 0;
              buffer = buffer->next;
            }
          }  /* while buffer */
      }  /* if num_soundings */
      else
        return 0;
    return (1);
}

/***************************************************************************/
/*!

  - Module Name:        get_cached_depth_records

  - Programmer(s):

  - Date Written:

  - Purpose:

  - Arguments:

  - Return Value:       1

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 get_cached_depth_records (NV_INT32 hnd, NV_I32_COORD2 coord, DEPTH_RECORD **depth, BIN_RECORD_SUMMARY **bsum)
{
    NV_INT32            i = 0, j;
    DEPTH_LIST         *buffer;
    NV_FLOAT32          x_offset, y_offset;

    depth_record_pos[hnd] = 0;

      buffer = &((*bsum)->depth.buffers);
      if ((*bsum)->num_soundings > 0) {
        *depth = (DEPTH_RECORD *) calloc ((*bsum)->num_soundings, sizeof(DEPTH_RECORD));
          while (buffer != NULL) {
#if 0
            if (!(i % hd[hnd].record_length)) {
              printf ("\nGET - RC = %ld, %ld I = %d Num = %ld: \n", coord.y, coord.x, i, bsum->num_soundings);
              for (k = 0; k < 131; k++) {
                if (!(k % 21) && (k > 0)) printf ("\n");
                printf (" %02x", buffer->depths[k]);
              }
              fflush(stdout);
            }
#endif
            (*depth)[i].coord.x = coord.x;
            (*depth)[i].coord.y = coord.y;
            
            /*  Unpack the current depth record from the 'physical' record.  */
            
            (*depth)[i].file_number = pfm_bit_unpack (buffer->depths, dep_off[hnd].file_number_pos + depth_record_pos[hnd],
                                                      hd[hnd].file_number_bits);
            
            (*depth)[i].line_number = pfm_bit_unpack (buffer->depths, dep_off[hnd].line_number_pos + depth_record_pos[hnd],
                                                      hd[hnd].line_number_bits);
            
            (*depth)[i].ping_number = pfm_bit_unpack (buffer->depths, dep_off[hnd].ping_number_pos + depth_record_pos[hnd],
                                                      hd[hnd].ping_number_bits);
            
            (*depth)[i].beam_number = pfm_bit_unpack (buffer->depths, dep_off[hnd].beam_number_pos + depth_record_pos[hnd],
                                                      hd[hnd].beam_number_bits);
            
            (*depth)[i].xyz.z = (NV_FLOAT32) (pfm_bit_unpack (buffer->depths, dep_off[hnd].depth_pos + depth_record_pos[hnd],
                                                              hd[hnd].depth_bits)) / hd[hnd].depth_scale - hd[hnd].depth_offset;
            
            /*  Stored as lat/lon.  */
            if (!hd[hnd].head.proj_data.projection)
            {
                x_offset = ((NV_FLOAT32) (pfm_bit_unpack (buffer->depths, dep_off[hnd].x_offset_pos + depth_record_pos[hnd],
                                                          hd[hnd].offset_bits)) / x_offset_scale[hnd]) * bin_header[hnd].x_bin_size_degrees;

                y_offset = ((NV_FLOAT32) (pfm_bit_unpack (buffer->depths, dep_off[hnd].y_offset_pos + depth_record_pos[hnd],
                                                          hd[hnd].offset_bits)) / y_offset_scale[hnd]) * bin_header[hnd].y_bin_size_degrees;
            
                /*  Compute the geographic position of the point. */
            
                (*depth)[i].xyz.y = bin_header[hnd].mbr.min_y + coord.y * bin_header[hnd].y_bin_size_degrees + y_offset;
                (*depth)[i].xyz.x = bin_header[hnd].mbr.min_x + coord.x * bin_header[hnd].x_bin_size_degrees + x_offset;
            }
            
            /*  Stored as x/y.  */
            else
            {
                x_offset = ((NV_FLOAT32) (pfm_bit_unpack (buffer->depths, dep_off[hnd].x_offset_pos + depth_record_pos[hnd],
                                                          hd[hnd].offset_bits)) / x_offset_scale[hnd]) * bin_header[hnd].bin_size_xy;
            
                y_offset = ((NV_FLOAT32) (pfm_bit_unpack (buffer->depths, dep_off[hnd].y_offset_pos + depth_record_pos[hnd],
                                                          hd[hnd].offset_bits)) / y_offset_scale[hnd]) * bin_header[hnd].bin_size_xy;
            
            
                /*  Compute the x/y position of the point. */
            
                (*depth)[i].xyz.y = bin_header[hnd].mbr.min_y + coord.y * bin_header[hnd].bin_size_xy + y_offset;
                (*depth)[i].xyz.x = bin_header[hnd].mbr.min_x + coord.x * bin_header[hnd].bin_size_xy + x_offset;
            }
            
            /*  Pre 4.0 version dependency.  */
            if (list_file_ver[hnd] < 40)
            {
                (*depth)[i].line_number = (*depth)[i].file_number;
            
                (*depth)[i].validity = pfm_bit_unpack (buffer->depths, dep_off[hnd].validity_pos + depth_record_pos[hnd], hd[hnd].validity_bits);
            }
            else
            {
                for (j = 0 ; j < hd[hnd].head.num_ndx_attr ; j++)
                  {
                    (*depth)[i].attr[j] = (NV_FLOAT32) (pfm_bit_unpack (buffer->depths, dep_off[hnd].attr_pos[j] +
                                                                        depth_record_pos[hnd], hd[hnd].ndx_attr_bits[j])) /
                      hd[hnd].head.ndx_attr_scale[j] + hd[hnd].head.min_ndx_attr[j];
                  }
            
                (*depth)[i].validity = pfm_bit_unpack (buffer->depths, dep_off[hnd].validity_pos + depth_record_pos[hnd],
                                                       hd[hnd].validity_bits);
            }
            if (hd[hnd].horizontal_error_bits)
              {
                (*depth)[i].horizontal_error = (NV_FLOAT32) (pfm_bit_unpack (buffer->depths, dep_off[hnd].horizontal_error_pos +
                                                                             depth_record_pos[hnd], hd[hnd].horizontal_error_bits)) /
                  hd[hnd].head.horizontal_error_scale;
                if ((*depth)[i].horizontal_error >= hd[hnd].horizontal_error_null) (*depth)[i].horizontal_error = -999.0;
              }
            
            if (hd[hnd].vertical_error_bits)
              {
                (*depth)[i].vertical_error = (NV_FLOAT32) (pfm_bit_unpack (buffer->depths, dep_off[hnd].vertical_error_pos +
                                                                           depth_record_pos[hnd], hd[hnd].vertical_error_bits)) /
                  hd[hnd].head.vertical_error_scale;
                if ((*depth)[i].vertical_error >= hd[hnd].vertical_error_null) (*depth)[i].vertical_error = -999.0;
              }
            
            depth_record_pos[hnd] += dep_off[hnd].single_point_bits;

            i++;
            if (i >= (*bsum)->num_soundings) {
              break;
            }
            if ((i > 0) && !(i % hd[hnd].record_length)) {
              depth_record_pos[hnd] = 0;
              buffer = buffer->next;
            }
          }  /* while buffer */
      }  /* if num_soundings */
      else
        return 0;
    return (1);
}


/***************************************************************************/
/*!

  - Module Name:        dump_cached_record

  - Programmer(s):

  - Date Written:

  - Purpose:            Print out the contents of a cached record.

  - Arguments:
                        - hnd             =   PFM file handle
                        - coord           =   COORD2 structure

  - Return Value:
                        - SUCCESS

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 dump_cached_record (NV_INT32 hnd, NV_I32_COORD2 coord)
{
    NV_INT32            k;
    BIN_RECORD_SUMMARY *bsum;
    DEPTH_LIST         *buffer;

    if (bin_cache_rows[hnd] != NULL) {
        if (bin_cache_rows[hnd][coord.y] != NULL) {
            if (bin_cache_rows[hnd][coord.y][coord.x] != NULL) {
              bsum = bin_cache_rows[hnd][coord.y][coord.x];
#ifdef NVWIN3X
              printf ("DUMP: RC = %03d, %03d BSumRC = %03d, %03d NumSndgs = %2d  Dirty = %d  CovFlag = %3d Head = %6I64d Tail = %6I64d RecPos = %3I64d Depths = %p\n",
                coord.y, 
                coord.x, 
                bsum->coord.y, 
                bsum->coord.x, 
                (NV_INT32) bsum->num_soundings, 
                bsum->dirty, 
                (NV_INT32) bsum->cov_flag, 
                (NV_INT64) bsum->depth.chain.head,
                (NV_INT64) bsum->depth.chain.tail,
                (NV_INT64) bsum->depth.record_pos,
                (void *) bsum->depth.buffers.depths);
#else
              printf ("DUMP: RC = %03d, %03d BSumRC = %03d, %03d NumSndgs = %2d  Dirty = %d  CovFlag = %3d Head = %6lld Tail = %6lld RecPos = %3lld Depths = %p\n",
                coord.y, 
                coord.x, 
                bsum->coord.y, 
                bsum->coord.x, 
                (NV_INT32) bsum->num_soundings, 
                bsum->dirty, 
                (NV_INT32) bsum->cov_flag, 
                (NV_INT64) bsum->depth.chain.head,
                (NV_INT64) bsum->depth.chain.tail,
                (NV_INT64) bsum->depth.record_pos,
                (void *) bsum->depth.buffers.depths);
#endif

              /* dump the depth buffer bytes of a 6 depth physical record. */

              buffer = &(bsum->depth.buffers);
              while( buffer != NULL) {
                for (k = 0; k < 131; k++) {
                  if (!(k % 21) && (k > 0)) printf ("\n");
                  printf (" %02x", buffer->depths[k]);
                }
                buffer = buffer->next;
                printf ("\n");
              }
            }
        }
    }
    return (pfm_error = SUCCESS);
}

/***************************************************************************/
/*!

  - Module Name:        dump_cached_records

  - Programmer(s):

  - Date Written:

  - Purpose:            Print out the contents of all cached records.

  - Arguments:
                        - hnd             =   PFM file handle

  - Return Value:
                        - SUCCESS

  - Caveats:            This code was actually developed by IVS in 
                        2007.  It was later modified by SAIC.  The
                        suspected culprits are listed below ;-)
                            - Danny Neville (IVS)
                            - Graeme Sweet (IVS)
                            - William Burrow (IVS)
                            - Jeff Parker (SAIC)

****************************************************************************/

NV_INT32 dump_cached_records (NV_INT32 hnd)
{
    NV_INT32            i, j, k;
    BIN_RECORD_SUMMARY *bsum;
    FILE               *bsum_ptr;
    static NV_INT32    count = 0;
    DEPTH_LIST         *buffer;

    if (count) {
      if ((bsum_ptr = fopen ("bsum.out", "a")) == NULL) {
        printf ("Can't open bsum.out ...\n");
        fflush (stdout);
        return 0;
      }
    }
    else {
      if ((bsum_ptr = fopen ("bsum.out", "w")) == NULL) {
        printf ("Can't open bsum.out ...\n");
        fflush (stdout);
        return 0;
      }
    }
    count++;
    fprintf (bsum_ptr, "NEW DUMP %d ...\n", count);
    if (bin_cache_rows[hnd] != NULL) {
      for (i = 0; i < cache_max_rows; i++) {
        if (bin_cache_rows[hnd][i] != NULL) {
          for (j = 0; j < cache_max_cols; j++) {
            if (bin_cache_rows[hnd][i][j] != NULL) {
              bsum = bin_cache_rows[hnd][i][j];
#ifdef NVWIN3X
              fprintf (bsum_ptr, "RC = %03d, %03d  NumSndgs = %2d  Dirty = %d  CovFlag = %3d Head = %6I64d Tail = %6I64d RecPos = %3I64d Depths = %p\n",
                bsum->coord.y, 
                bsum->coord.x, 
                (NV_INT32) bsum->num_soundings, 
                bsum->dirty, 
                (NV_INT32) bsum->cov_flag, 
                (NV_INT64) bsum->depth.chain.head,
                (NV_INT64) bsum->depth.chain.tail,
                (NV_INT64) bsum->depth.record_pos,
                (void *) bsum->depth.buffers.depths);
#else
              fprintf (bsum_ptr, "RC = %03d, %03d  NumSndgs = %2d  Dirty = %d  CovFlag = %3d Head = %6lld Tail = %6lld RecPos = %3lld Depths = %p\n",
                bsum->coord.y, 
                bsum->coord.x, 
                (NV_INT32) bsum->num_soundings, 
                bsum->dirty, 
                (NV_INT32) bsum->cov_flag, 
                (NV_INT64) bsum->depth.chain.head,
                (NV_INT64) bsum->depth.chain.tail,
                (NV_INT64) bsum->depth.record_pos,
                (void *) bsum->depth.buffers.depths);
#endif

              /* dump the depth buffer bytes of a 6 depth physical record. */
              buffer = &(bsum->depth.buffers);
              if (bsum->num_soundings > 0) { 
                while( buffer != NULL) {
                  for (k = 0; k < 131; k++) {
                    if (!(k % 21) && (k > 0)) fprintf (bsum_ptr, "\n");
                    fprintf (bsum_ptr, " %02x", buffer->depths[k]);
                  }
                  buffer = buffer->next;
                  fprintf (bsum_ptr, "\n");
                  fflush (bsum_ptr);
                }
              }
            }
          }
        }
      }
    }
    fclose (bsum_ptr);

    return (pfm_error = SUCCESS);
}




