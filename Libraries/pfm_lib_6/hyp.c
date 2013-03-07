
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



/*! <pre>
 *****************************************************************************
 ***   Interactive Visualizations Systems Inc.                             ***
 *****************************************************************************
 * LIBRARY        : hyp                                                      *
 * AUTHOR(S)      : Graeme Sweet (modified by J. Parker - SAIC)              *
 * FILE           : hyp.c (cloned from pcube.c)                              *
 * DATE CREATED   : Jan 13th, 2004 (modified May 2006)                       *
 * PURPOSE        :                                                          *
 *  This library provides an interface to the hypothesis file of a PFM.      *
 *  A list of generated hypotheses is stored in a directory with extension   *
 *  ".hyp".                                                                  *
 *                                                                           *
 *****************************************************************************
 </pre>*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "hyp.h"
#include "pfm.h"
#include "huge_io.h"
#include "large_io.h"

/*****************************************************************************
 ***
 *** Hypothesis Constants and Static Variables
 ***
 *****************************************************************************/


/*! Hypothesis error status */
static  NV_CHAR                  hyp_err_str[512];

/*! Variables and functions for the progress callback */
static  HYP_PROGRESS_CALLBACK  hyp_progress_callback = NULL;


/*!  Static function arrays for I/O (either HUGE or LARGE).  */

static NV_INT32 (*pfm_fseek[2]) (NV_INT32 handle, NV_INT64 offset, NV_INT32 whence) = {lfseek, hfseek};
static NV_INT32 (*pfm_fopen[2]) (NV_CHAR *path, NV_CHAR *mode) = {lfopen, hfopen};
static size_t (*pfm_fread[2]) (void *ptr, size_t size, size_t nmemb, NV_INT32 handle) = {lfread, hfread};
static size_t (*pfm_fwrite[2]) (void *ptr, size_t size, size_t nmemb, NV_INT32 handle) = {lfwrite, hfwrite};
static NV_INT64 (*pfm_ftell[2]) (NV_INT32 handle) = {lftell, hftell};
static NV_INT32 (*pfm_fclose[2]) (NV_INT32 handle) = {lfclose, hfclose};
static void (*pfm_fflush[2]) (NV_INT32 handle) = {lfflush, hfflush};
static NV_INT32 pfm_io_type[MAX_PFM_FILES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


/*!  These defines make the code a bit more readable but you must make sure that what you are accessing is ALWAYS
     (*pfm_whatever[pfm_io_type[p->pfmhnd]]).  If not, use the long call.  */

#define PFM_FSEEK   (*pfm_fseek[pfm_io_type[p->pfmhnd]])
#define PFM_FOPEN   (*pfm_fopen[pfm_io_type[p->pfmhnd]])
#define PFM_FREAD   (*pfm_fread[pfm_io_type[p->pfmhnd]])
#define PFM_FWRITE  (*pfm_fwrite[pfm_io_type[p->pfmhnd]])
#define PFM_FTELL   (*pfm_ftell[pfm_io_type[p->pfmhnd]])
#define PFM_FCLOSE  (*pfm_fclose[pfm_io_type[p->pfmhnd]])
#define PFM_FFLUSH  (*pfm_fflush[pfm_io_type[p->pfmhnd]])



/*!  Big/Little endian checker.  */

static NV_INT32 hyp_big_endian ()
{
  union
  {
    NV_INT32        word;
    NV_U_BYTE       byte[4];
  } a;

  a.word = 0x00010203;
  return ((NV_INT32) a.byte[3]);
}


static NV_INT32 hyp_endian = 0;


/*****************************************************************************
 ***
 *** Hypothesis Private Function Prototypes
 ***
 *****************************************************************************/

static NV_INT32   hyp_fwrite( void *buffer, NV_INT32 size, NV_INT32 count, HypHead *p );
static NV_INT32   hyp_fread( void *buffer, NV_INT32 size, NV_INT32 count, HypHead *p );
static void       hyp_swap_bytes( void *buffer, NV_INT32 size, NV_INT32 count, NV_INT32 endian );
static void       hyp_free_node_data( HypNode *node );

/*****************************************************************************
 *****************************************************************************
 ***
 *** Hypothesis Public Functions
 ***
 *****************************************************************************
 *****************************************************************************/

/*! <pre>
 *******************************************************************************
 * Function  : hyp_get_node
 * Arguments :
 * Returns   :
 * Purpose   : Returns a pointer to a Cube Node at the given row and column
 *   position. The caller is responsible for freeing the object using
 *  'hyp_free_node'.
 *******************************************************************************
 </pre>*/
HypNode*   hyp_get_node( HypHead *p, NV_INT32 x, NV_INT32 y )
{
    /* Allocate node and read from disk */
    HypNode *n = (HypNode*)malloc( sizeof(HypNode) );
    memset( n, 0, sizeof(HypNode) );
    hyp_read_node( p, x, y, n );
    return n;
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_free_node
 * Arguments :
 * Returns   :
 * Purpose   : Frees a node returned by hyp_get_node.
 *******************************************************************************
 </pre>*/
void         hyp_free_node( HypNode *node )
{
    /* Delete memory used by node */
    hyp_free_node_data( node );
    free( node );
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_register_progress_callback
 * Arguments :
 * Returns   :
 * Purpose   : Supply a progress callback function to be called when recubing
 *     an area.
 *******************************************************************************
 </pre>*/
void         hyp_register_progress_callback( HYP_PROGRESS_CALLBACK progressCB )
{
    hyp_progress_callback = progressCB;
}

/*****************************************************************************
 *****************************************************************************
 ***
 *** Hypothesis Private Functions
 ***
 *****************************************************************************
 *****************************************************************************/

/*! <pre>
 *******************************************************************************
 * Function  : hyp_form_hyp_filename
 * Arguments :
 * Returns   :
 * Purpose   : Forms the filename for the hyp file.
 *******************************************************************************
 </pre>*/

static NV_INT32     hyp_form_hyp_filename( char *list_file_path, char *file )
{
  char    hyp_name[256];
  char    data_name[256];
  int     i;

/*
  for (i = (strlen(list_file_path) - 1); i > 0; i--) {
    if (list_file_path[i] == '.') {
      list_file_path[i] = 0;
      break;
    }
  }
*/
  sprintf (data_name, "%s.data", list_file_path);
  sprintf (hyp_name, "%s.hyp", list_file_path);
  for (i = (strlen(hyp_name) - 1); i > 0; i--) {
    if ((hyp_name[i] == 47) || (hyp_name[i] == 92)) {
      break;
    }
  }
  memcpy (&hyp_name[0], &hyp_name[i], strlen(hyp_name) - i + 1);

  if (access (data_name, F_OK) == 0) {
    sprintf (file, "%s/%s", data_name, hyp_name);
  }
  else {
    sprintf (file, "%s.hyp", list_file_path);
  }

  return 1;
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_create_file
 * Arguments :
 * Returns   :
 * Purpose   : Creates a new hypothesis file with extension ".hyp". Associates the
 *   structure with an open handle to a PFM file. The HYP data structure
 *   built will be the same size as the existing PFM file. The attributes tell
 *   the library where to find information from the PFM file. The horizontal
 *   and vertical error attributes are required, but the others can be disabled
 *   by passing -1.
 *******************************************************************************
 </pre>*/
HypHead*       hyp_create_file( char *list_file_path, NV_INT32 pfmhnd,
                                NV_INT32 horizErrorAttr, NV_INT32 vertErrorAttr,
                                NV_INT32 numHyposAttr, NV_INT32 hypoStrengthAttr,
                                NV_INT32 uncertaintyAttr, NV_INT32 customHypoFlag )
{
    HypHead   *p;
    char       filename[256], *data;
    NV_INT32   hnd, i, j, percent, old_percent;

    /* Form the filename for the .hyp file */
    if( hyp_form_hyp_filename( list_file_path, filename ) < 0 )
    {
        fprintf(stderr, "Could not locate information from PFM list file '%s'.", list_file_path );
        return NULL;
    }


    /*  Get the I/O type from the PFM API for this PFM handle.  */

    pfm_io_type[pfmhnd] = pfm_get_io_type (pfmhnd);


    /* Try to open hugeio file */
    hnd = (*pfm_fopen[pfm_io_type[pfmhnd]])( filename, "w+b" );
    if( hnd == -1 )
    {
        sprintf( hyp_err_str, "Error creating file in '%s'.", filename );
        return NULL;
    }

    /* Create new object and initialize */
    p = (HypHead *)calloc(1, sizeof(HypHead));
    memset( p, 0, sizeof(HypHead) );
    p->cube_param = (CubeParam *) calloc (1, sizeof(CubeParam));
    read_bin_header( pfmhnd, &p->bin_header );
    p->version            = HYP_VERSION;
    p->pfmhnd             = pfmhnd;
    p->ncols              = p->bin_header.bin_width;
    p->nrows              = p->bin_header.bin_height;
    p->filehnd            = hnd;
    p->readonly           = NVFalse;

    if (hyp_big_endian ())
      {
        hyp_endian = p->endian = HYP_FTYPE_BIGENDIAN;
      }
    else
      {
        hyp_endian = p->endian = HYP_FTYPE_LITENDIAN;
      }

    p->vert_error_attr    = vertErrorAttr;
    p->horiz_error_attr   = horizErrorAttr;
    p->num_hypos_attr     = numHyposAttr;
    p->hypo_strength_attr = hypoStrengthAttr;
    p->uncertainty_attr   = uncertaintyAttr;
    p->custom_hypo_flag   = customHypoFlag;
    p->deleted_ptr        = 0;
    p->file_size          = HYP_ASCII_HEADER_SIZE + HYP_BINARY_HEADER_SIZE + ((NV_INT64)p->ncols * (NV_INT64)p->nrows * HYP_NODE_RECORD_SIZE);

    /* Create the Cube parameters */
/*
    p->cube_param      = cube_init_param( HYP_DEFAULT_IHO_ORDER, p->bin_header.bin_size_xy, p->bin_header.bin_size_xy );
    p->hypo_resolve_algo    = p->cube_param->mthd;
    p->node_capture_percent = p->cube_param->capture_dist_scale * 100.0f;
*/
    /* Write the header information to the file */
    hyp_write_header( p );

    /* Write empty information for the Nodes */
    PFM_FSEEK( p->filehnd, HYP_ASCII_HEADER_SIZE + HYP_BINARY_HEADER_SIZE, SEEK_SET );
    data        = (char*)malloc( HYP_NODE_RECORD_SIZE );
    memset( data, 0, HYP_NODE_RECORD_SIZE );
    old_percent = 0;
    for( i = 0; i < p->nrows; i++ )
    {
        /* Write row data */
        for( j = 0; j < p->ncols; j++ )
        {
          PFM_FWRITE( (void*)data, HYP_NODE_RECORD_SIZE, 1, p->filehnd );
        }

        /* Update status callback */
        percent = (NV_INT32)( ((NV_FLOAT32)i / p->nrows) * 100.0 );
        if (percent != old_percent)
        {
            /*  Calls a status callback if register. */
            if (hyp_progress_callback)
                (*hyp_progress_callback) (1, percent);
            old_percent = percent;
        }
    }
    free( data );

    /* Return the created HypHead object */
    return p;
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_open_file
 * Arguments :
 * Returns   :
 * Purpose   : Attempts to open a hypothesis file that is associated with the opened
 *   PFM specified by the PFM handle. NULL is returned if the file ".hyp"
 *   could not be found.
 *******************************************************************************
 </pre>*/
HypHead*       hyp_open_file( char *list_file_path, NV_INT32 pfmhnd )
{
    HypHead   *p;
    char       filename[256];

    /* Form the filename for the .hyp file */
    if( hyp_form_hyp_filename( list_file_path, filename ) < 0 )
    {
        sprintf( hyp_err_str, "Could not locate information from PFM list file '%s'.", list_file_path );
        return NULL;
    }


    /*  Get the I/O type from the PFM API for this PFM handle.  */

    pfm_io_type[pfmhnd] = pfm_get_io_type (pfmhnd);


    /* Create HypHead object */
    p = (HypHead*)calloc(1, sizeof(HypHead));
    memset( p, 0, sizeof(HypHead) );

    /* Try to open hugeio file */
    p->pfmhnd  = pfmhnd;
    p->filehnd = PFM_FOPEN( filename, "r+b" );
    if( p->filehnd == -1 )
    {
        p->filehnd = PFM_FOPEN( filename, "rb" );
        p->readonly = NVTrue;
    }
    if( p->filehnd == -1 )
    {
        sprintf( hyp_err_str, "Could not load cube hypotheses from PFM file '%s'.", list_file_path );
        if (p) {
          free( p );
          p = NULL;
        }
        return NULL;
    }
    p->cube_param = (CubeParam *) calloc (1, sizeof(CubeParam));

    /* Read bin header from pfm file */
    read_bin_header( pfmhnd, &p->bin_header );

    /* Create the Cube parameters */
/*
    p->cube_param = cube_init_param( HYP_DEFAULT_IHO_ORDER, p->bin_header.bin_size_xy, p->bin_header.bin_size_xy );
    p->hypo_resolve_algo    = p->cube_param->mthd;
    p->node_capture_percent = p->cube_param->capture_dist_scale * 100.0f;
*/
    /* Read the header information from the file */
    hyp_read_header( p );

    /* Check that hyp version is proper */
    if( p->version < 1.0 || p->version > HYP_VERSION )
    {
        sprintf( hyp_err_str, "Hypothesis file had unsupported version '%.2f'. This file may have been created with a newer version of the software.", p->version );

        if( p->cube_param )
          free (p->cube_param);
/*
            cube_release_param( p->cube_param );
*/

        if (p) {
          free( p );
          p = NULL;
        }
        return NULL;
    }

    /* Check that hyp size is proper */
    if( (p->ncols != p->bin_header.bin_width) ||
        (p->nrows != p->bin_header.bin_height) )
    {
        sprintf( hyp_err_str, "Hypothesis file did not match PFM file in size." );

        if (p->cube_param ) {
          free (p->cube_param);
          p->cube_param = NULL;
        }
/*
            cube_release_param( p->cube_param );
*/
        if (p) {
          free( p );
          p = NULL;
        }
        return NULL;
    }

    /* Make sure file size matches expected */
    /*
    PFM_FSEEK( p->filehnd, 0, SEEK_END );
    if( PFM_FTELL( p->filehnd ) != p->file_size )
    {
        sprintf( hyp_err_str, "Hypothesis file size is not correct. File may have been corrupted." );
        free( p );
        return NULL;
    }*/

    /* Return created HypHead object */
    return p;
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_close_file
 * Arguments :
 * Returns   :
 * Purpose   : Closes the hypothesis file that was opened with hyp_create_file or
 *   hyp_open_file. Frees any memory used by the objects.
 *******************************************************************************
 </pre>*/
void         hyp_close_file( HypHead *p )
{
    /* Free the parameters */
/*
    cube_release_param( p->cube_param );
*/

    if (p->cube_param ) {
      free (p->cube_param);
      p->cube_param = NULL;
    }

    /* Delete memory used by hyp */
/*
    if (p) {
      free( p );
      p = NULL;
    }
*/
    /* Close file handle */
    PFM_FCLOSE( p->filehnd );
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_write_header
 * Arguments :
 * Returns   :
 * Purpose   : Writes the hyp header to the disk.
 *******************************************************************************
 </pre>*/

NV_INT32     hyp_write_header( HypHead *p )
{
    char buf[HYP_ASCII_HEADER_SIZE];
    int  write_count = 0;

    /* Make sure the version of the header we are writing is up to date */
    p->version = HYP_VERSION;

    /* Write the ASCII header */
    PFM_FSEEK( p->filehnd, 0, SEEK_SET );
    memset( buf, 0, HYP_ASCII_HEADER_SIZE );
    sprintf( buf, "Hypothesis Library - Version %.2f\n", p->version );
    if (hyp_fwrite( buf, 1, HYP_ASCII_HEADER_SIZE, p ) < 1)
      {
        printf ("ERROR:  writing HYP header failed ...\n");
        return -1;
      }

    /* Write the Binary header */
    memset( buf, 0, HYP_BINARY_HEADER_SIZE );
    write_count += hyp_fwrite( buf, 1, HYP_BINARY_HEADER_SIZE, p );
    PFM_FSEEK( p->filehnd, HYP_ASCII_HEADER_SIZE, SEEK_SET );
    write_count += hyp_fwrite( &p->endian,                          sizeof(NV_U_CHAR), 1, p );
    PFM_FSEEK( p->filehnd, HYP_DELETED_POINTER_POS, SEEK_SET );
    write_count += hyp_fwrite( &p->deleted_ptr,                     sizeof(NV_INT64),  1, p );
    write_count += hyp_fwrite( &p->file_size,                       sizeof(NV_INT64),  1, p );
    write_count += hyp_fwrite( &p->ncols,                           sizeof(NV_INT32),  1, p );
    write_count += hyp_fwrite( &p->nrows,                           sizeof(NV_INT32),  1, p );
    write_count += hyp_fwrite( &p->vert_error_attr,                 sizeof(NV_INT32),  1, p );
    write_count += hyp_fwrite( &p->horiz_error_attr,                sizeof(NV_INT32),  1, p );
    write_count += hyp_fwrite( &p->num_hypos_attr,                  sizeof(NV_INT32),  1, p );
    write_count += hyp_fwrite( &p->hypo_strength_attr,              sizeof(NV_INT32),  1, p );
    write_count += hyp_fwrite( &p->uncertainty_attr,                sizeof(NV_INT32),  1, p );
    write_count += hyp_fwrite( &p->custom_hypo_flag,                sizeof(NV_INT32),  1, p );
    write_count += hyp_fwrite( &p->hypo_resolve_algo,               sizeof(NV_INT32),  1, p );
    write_count += hyp_fwrite( &p->node_capture_percent,            sizeof(NV_FLOAT32),  1, p );

    write_count += hyp_fwrite( &p->cube_param->init,                sizeof(Bool), 1, p );
    write_count += hyp_fwrite( &p->cube_param->no_data_value,       sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->mthd,                sizeof(CubeExtractor), 1, p );
    write_count += hyp_fwrite( &p->cube_param->null_depth,          sizeof(NV_FLOAT64), 1, p );
    write_count += hyp_fwrite( &p->cube_param->null_variance,       sizeof(NV_FLOAT64), 1, p );
    write_count += hyp_fwrite( &p->cube_param->dist_exp,            sizeof(NV_FLOAT64), 1, p );
    write_count += hyp_fwrite( &p->cube_param->inv_dist_exp,        sizeof(NV_FLOAT64), 1, p );
    write_count += hyp_fwrite( &p->cube_param->dist_scale,          sizeof(NV_FLOAT64), 1, p );
    write_count += hyp_fwrite( &p->cube_param->var_scale,           sizeof(NV_FLOAT64), 1, p );
    write_count += hyp_fwrite( &p->cube_param->iho_fixed,           sizeof(NV_FLOAT64), 1, p );
    write_count += hyp_fwrite( &p->cube_param->iho_pcent,           sizeof(NV_FLOAT64), 1, p );
    write_count += hyp_fwrite( &p->cube_param->median_length,       sizeof(NV_U_INT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->quotient_limit,      sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->discount,            sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->est_offset,          sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->bayes_fac_t,         sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->runlength_t,         sizeof(NV_U_INT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->min_context,         sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->max_context,         sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->sd2conf_scale,       sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->blunder_min,         sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->blunder_pcent,       sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->blunder_scalar,      sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &p->cube_param->capture_dist_scale,  sizeof(NV_FLOAT32), 1, p );

    if (write_count < 38)
      {
        printf ("ERROR: writing HYP header failed.\n");
        return -1;
      }

    return 0;
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_read_header
 * Arguments :
 * Returns   :
 * Purpose   : Reads the hyp header from disk.
 *******************************************************************************
 </pre>*/
NV_INT32     hyp_read_header( HypHead *p )
{
    char buf[HYP_ASCII_HEADER_SIZE];
    int  read_count = 0;

    if (p->filehnd <= 0) {
      return -1;
    }
    /* Read ASCII header and extract version */
    PFM_FSEEK( p->filehnd, 0, SEEK_SET );
    memset( buf, 0, HYP_ASCII_HEADER_SIZE );
    if (hyp_fread( buf, 1, HYP_ASCII_HEADER_SIZE, p ) < 1)
      {
        printf ("ERROR:  reading HYP header failed ...\n");
        return -1;
      }


    /*  Check for the IVS PCube style header as well as the standard header.  */

    if (strstr (buf, "PCube"))
      {
	sscanf( buf, "PCube Library - Version %f", &p->version );
      }
    else
      {
	sscanf( buf, "Hypothesis Library - Version %f", &p->version );
      }

    /* Read Binary header */
    PFM_FSEEK( p->filehnd, HYP_ASCII_HEADER_SIZE, SEEK_SET );
    read_count += hyp_fread( &p->endian,                          sizeof(NV_U_CHAR), 1, p );
    PFM_FSEEK( p->filehnd, HYP_DELETED_POINTER_POS, SEEK_SET );
    read_count += hyp_fread( &p->deleted_ptr,                     sizeof(NV_INT64),  1, p );
    read_count += hyp_fread( &p->file_size,                       sizeof(NV_INT64),  1, p );
    read_count += hyp_fread( &p->ncols,                           sizeof(NV_INT32),  1, p );
    read_count += hyp_fread( &p->nrows,                           sizeof(NV_INT32),  1, p );

    if (read_count < 5)
      {
        printf ("ERROR:  reading Deleted Pointer Failed ...\n");
        return -1;
      }


    /*  Checking to see if we may have the wrong endian in the header.  */

    if ((p->nrows != p->bin_header.bin_height) || (p->ncols != p->bin_header.bin_width))
      {
        int tmp_count = 0;

        if (p->endian)
          {
            p->endian = 0;
          }
        else
          {
            p->endian = 1;
          }
        hfseek( p->filehnd, HYP_DELETED_POINTER_POS, SEEK_SET );
        tmp_count += hyp_fread( &p->deleted_ptr,                     sizeof(NV_INT64),  1, p );
        tmp_count += hyp_fread( &p->file_size,                       sizeof(NV_INT64),  1, p );
        tmp_count += hyp_fread( &p->ncols,                           sizeof(NV_INT32),  1, p );
        tmp_count += hyp_fread( &p->nrows,                           sizeof(NV_INT32),  1, p );
        if (tmp_count < 4)
          {
            printf ("ERROR:  reading Deleted Pointer Failed ...\n");
            return -1;
          }
    }

    read_count += hyp_fread( &p->vert_error_attr,                 sizeof(NV_INT32),  1, p );
    read_count += hyp_fread( &p->horiz_error_attr,                sizeof(NV_INT32),  1, p );
    read_count += hyp_fread( &p->num_hypos_attr,                  sizeof(NV_INT32),  1, p );
    read_count += hyp_fread( &p->hypo_strength_attr,              sizeof(NV_INT32),  1, p );
    read_count += hyp_fread( &p->uncertainty_attr,                sizeof(NV_INT32),  1, p );
    read_count += hyp_fread( &p->custom_hypo_flag,                sizeof(NV_INT32),  1, p );
    read_count += hyp_fread( &p->hypo_resolve_algo,               sizeof(NV_INT32),  1, p );

    read_count += hyp_fread( &p->node_capture_percent,            sizeof(NV_FLOAT32),  1, p );
    read_count += hyp_fread( &p->cube_param->init,                sizeof(Bool), 1, p );
    read_count += hyp_fread( &p->cube_param->no_data_value,       sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &p->cube_param->mthd,                sizeof(CubeExtractor), 1, p );
    read_count += hyp_fread( &p->cube_param->null_depth,          sizeof(NV_FLOAT64), 1, p );
    read_count += hyp_fread( &p->cube_param->null_variance,       sizeof(NV_FLOAT64), 1, p );
    read_count += hyp_fread( &p->cube_param->dist_exp,            sizeof(NV_FLOAT64), 1, p );
    read_count += hyp_fread( &p->cube_param->inv_dist_exp,        sizeof(NV_FLOAT64), 1, p );
    read_count += hyp_fread( &p->cube_param->dist_scale,          sizeof(NV_FLOAT64), 1, p );
    read_count += hyp_fread( &p->cube_param->var_scale,           sizeof(NV_FLOAT64), 1, p );
    read_count += hyp_fread( &p->cube_param->iho_fixed,           sizeof(NV_FLOAT64), 1, p );
    read_count += hyp_fread( &p->cube_param->iho_pcent,           sizeof(NV_FLOAT64), 1, p );
    read_count += hyp_fread( &p->cube_param->median_length,       sizeof(NV_U_INT32), 1, p );
    read_count += hyp_fread( &p->cube_param->quotient_limit,      sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &p->cube_param->discount,            sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &p->cube_param->est_offset,          sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &p->cube_param->bayes_fac_t,         sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &p->cube_param->runlength_t,         sizeof(NV_U_INT32), 1, p );
    read_count += hyp_fread( &p->cube_param->min_context,         sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &p->cube_param->max_context,         sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &p->cube_param->sd2conf_scale,       sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &p->cube_param->blunder_min,         sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &p->cube_param->blunder_pcent,       sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &p->cube_param->blunder_scalar,      sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &p->cube_param->capture_dist_scale,  sizeof(NV_FLOAT32), 1, p );

    if (read_count < 32)
      {
        printf ("ERROR:  reading HYP header failed ...\n");
        return -1;
      }

    return 0;
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_read_node
 * Arguments :
 * Returns   :
 * Purpose   : Reads the given node from the disk.
 *******************************************************************************
 </pre>*/
NV_INT32    hyp_read_node( HypHead *p, NV_INT32 x, NV_INT32 y, HypNode *node )
{
    NV_INT64     ptr;
    NV_INT32     count, read_count = 0;
    PHypothesis *h;

    /* Read node information from disk */
    PFM_FSEEK( p->filehnd, HYP_ASCII_HEADER_SIZE + HYP_BINARY_HEADER_SIZE + ((NV_INT64)y * (NV_INT64)p->ncols + (NV_INT64)x) * HYP_NODE_RECORD_SIZE, SEEK_SET );
    read_count += hyp_fread( &node->best_hypo, sizeof(NV_FLOAT32), 1, p );
    read_count += hyp_fread( &node->n_hypos,   sizeof(NV_INT32),   1, p );
    read_count += hyp_fread( &ptr,             sizeof(NV_INT64),   1, p );
    if (read_count < 3)
      {
        printf ("ERROR:  reading HYP node failed ...\n");
        return -1;
      }

    /* Allocate the hypotheses */
    if( node->hypos )
        free( node->hypos );
    if( node->n_hypos == 0 )
        node->hypos = NULL;
    else
        node->hypos = (PHypothesis*)malloc( node->n_hypos * sizeof(PHypothesis) );

    /* Read all hypotheses from the disk */
    count = 0;
    while( ptr && (count < node->n_hypos) )
    {
        /* Read a hypothesis */
        read_count = 0;
        PFM_FSEEK( p->filehnd, ptr, SEEK_SET );
        h = &node->hypos[ count ];
        read_count += hyp_fread( &h->z,      sizeof(NV_FLOAT32), 1, p );
        read_count += hyp_fread( &h->var,    sizeof(NV_FLOAT32), 1, p );
        read_count += hyp_fread( &h->n_snds, sizeof(NV_INT32),   1, p );
        read_count += hyp_fread( &h->avg_tpe,sizeof(NV_FLOAT32), 1, p );
        read_count += hyp_fread( &ptr,       sizeof(NV_INT64),   1, p );
        h->ci = (NV_FLOAT32)(p->cube_param->sd2conf_scale * sqrt(h->var ));
        count++;
        if (read_count < 5)
          {
            printf ("ERROR:  reading HYP node failed ...\n");
            return -1;
          }
    }

    /* Check for inconsistent read */
    if( ptr || (count < node->n_hypos) )
        return -1;
    else
        return 0;
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_write_node
 * Arguments :
 * Returns   :
 * Purpose   : Writes the given node to the disk.
 *******************************************************************************
 </pre>*/
NV_INT32    hyp_write_node( HypHead *p, NV_INT32 x, NV_INT32 y, HypNode *node )
{
    NV_INT64     ptr, address, deleted_ptr, deleted_address = 0, zero, old_deleted_ptr, old_file_size;
    NV_INT32     i, new_record, write_count = 0;
    PHypothesis *h;

    /* Read node information from disk */
    old_deleted_ptr = p->deleted_ptr;
    old_file_size   = p->file_size;
    PFM_FFLUSH( p->filehnd );
    PFM_FSEEK( p->filehnd, HYP_ASCII_HEADER_SIZE + HYP_BINARY_HEADER_SIZE + ((NV_INT64)y * (NV_INT64)p->ncols + (NV_INT64)x) * HYP_NODE_RECORD_SIZE, SEEK_SET );
    write_count += hyp_fwrite( &node->best_hypo, sizeof(NV_FLOAT32), 1, p );
    write_count += hyp_fwrite( &node->n_hypos,   sizeof(NV_INT32),   1, p );
    if (write_count < 2)
      {
        printf ("ERROR: writing HYP node failed ...\n");
        return -1;
      }
    address = PFM_FTELL( p->filehnd );
    PFM_FFLUSH( p->filehnd );
    hyp_fread ( &ptr,             sizeof(NV_INT64),   1, p );

    /* Write all hypotheses to the disk */
    for( i = 0; i < node->n_hypos; i++ )
    {
        /* Get address for new hypothesis */
        new_record = NVFalse;
        if( ptr == 0 )
        {
            /* Find address for new record */
            if( p->deleted_ptr )
            {
                /* Get new record from list of deleted records */
                ptr = p->deleted_ptr;
                PFM_FFLUSH( p->filehnd );
                PFM_FSEEK( p->filehnd, ptr + HYP_POINTER_POS, SEEK_SET );
                hyp_fread( &p->deleted_ptr, sizeof(NV_INT64), 1, p ); /* Slide deleted pointer ahead */
                PFM_FFLUSH( p->filehnd );
                new_record = NVTrue;
            }
            else
            {
                /* Get new record from the end of the file */
                ptr = p->file_size;
                p->file_size += HYP_HYPOTHESIS_RECORD_SIZE;
                new_record = NVTrue;
            }

            /* Write address */
            PFM_FSEEK( p->filehnd, address, SEEK_SET );
            hyp_fwrite( &ptr, sizeof(NV_INT64), 1, p );
        }

        /* Write a hypothesis */
        h = &node->hypos[ i ];
        write_count = 0;
        PFM_FSEEK( p->filehnd, ptr, SEEK_SET );
        write_count += hyp_fwrite( &h->z,      sizeof(NV_FLOAT32), 1, p );
        write_count += hyp_fwrite( &h->var,    sizeof(NV_FLOAT32), 1, p );
        write_count += hyp_fwrite( &h->n_snds, sizeof(NV_INT32),   1, p );
        write_count += hyp_fwrite( &h->avg_tpe,sizeof(NV_FLOAT32), 1, p );
        if (write_count < 4)
          {
            printf ("ERROR: writing HYP node failed ...\n");
            return -1;
          }
        address = PFM_FTELL( p->filehnd );
        PFM_FFLUSH( p->filehnd );
        if( new_record )
          {
            ptr = 0;
          }
        else
          {
            if (hyp_fread ( &ptr,   sizeof(NV_INT64),   1, p ) < 1)
              {
                printf ("ERROR:  reading ptr from node failed ...\n");
                return -1;
              }
          }

        if (ptr && (i >= (node->n_hypos - 1))) {
          ptr = 0;
        }

    }  /* for i */

    /* Add all records left over onto the deleted records list */
    if( ptr )
    {
        /* Read to end of the chain */
        PFM_FFLUSH( p->filehnd );
        deleted_ptr = ptr;
        while( deleted_ptr )
        {
            deleted_address = deleted_ptr + HYP_POINTER_POS;
            PFM_FSEEK( p->filehnd, deleted_address, SEEK_SET );
            if (hyp_fread( &deleted_ptr, sizeof(NV_INT64), 1, p ) < 1)
              {
                printf ("ERROR:  reading delted_ptr from node failed ...\n");
                return -1;
              }
        }

        /* Connect end of chain to the deleted record list */
        PFM_FSEEK( p->filehnd, deleted_address, SEEK_SET );
        PFM_FFLUSH( p->filehnd );
        if (hyp_fwrite( &p->deleted_ptr, sizeof(NV_INT64), 1, p ) < 1)
          {
            printf ("ERROR:  writingdeleted_ptr to node failed ...\n");
            return -1;
          }

        /* Change the head of the deleted chain */
        p->deleted_ptr = ptr;
    }

    /* Make sure that the good chain ends with a zero */
    zero = 0;
    PFM_FFLUSH( p->filehnd );
    PFM_FSEEK( p->filehnd, address, SEEK_SET );
    if (hyp_fwrite( &zero, sizeof(NV_INT64), 1, p ) < 1)
      {
        printf ("ERROR:  writing zero to node failed ...\n");
        return -1;
      }

    /* Write deleted pointer if necessary - keep file consistent */
    if( old_deleted_ptr != p->deleted_ptr )
    {
        PFM_FSEEK( p->filehnd, HYP_DELETED_POINTER_POS, SEEK_SET );
        if (hyp_fwrite( &p->deleted_ptr, sizeof(NV_INT64), 1, p ) < 1)
          {
            printf ("ERROR:  writing deleted_ptr to node failed ...\n");
            return -1;
          }
    }

    /* Write file size if necessary - keep file consistent */
    if( old_file_size != p->file_size )
    {
        PFM_FSEEK( p->filehnd, HYP_FILE_SIZE_POS, SEEK_SET );
        if (hyp_fwrite( &p->file_size, sizeof(NV_INT64), 1, p ) < 1)
          {
            printf ("ERROR:  writing filesize to node failed ...\n");
            return -1;
          }
    }

    PFM_FFLUSH( p->filehnd );
    return 0;
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_fwrite
 * Arguments :
 * Returns   :
 * Purpose   : Writes data to disk, swapping for endian if necessary.
 *******************************************************************************
 </pre>*/
static NV_INT32     hyp_fwrite( void *buffer, NV_INT32 size, NV_INT32 count, HypHead *p )
{
    NV_INT32 writeStatus;

    /* Swap if necessary */
    hyp_swap_bytes( buffer, size, count, p->endian );

    /* Write data */
    writeStatus = PFM_FWRITE( buffer, size, count, p->filehnd );
    /*  NOTE:  hfwrite return 0 for failure to >0 for success. */

    /* Swap back */
    hyp_swap_bytes( buffer, size, count, p->endian );
    return writeStatus;
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_fread
 * Arguments :
 * Returns   :
 * Purpose   : Writes data to disk, swapping for endian if necessary.
 *******************************************************************************
 </pre>*/
static NV_INT32     hyp_fread( void *buffer, NV_INT32 size, NV_INT32 count, HypHead *p )
{
    NV_INT32 readStatus;

    /* Read data */
    readStatus = PFM_FREAD( buffer, size, count, p->filehnd );

    /* Swap if necessary */
    hyp_swap_bytes( buffer, size, count, p->endian );
    return readStatus;
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_swap_bytes
 * Arguments :
 * Returns   :
 * Purpose   : Swaps bytes only if necessary.
 *******************************************************************************
 </pre>*/
static void    hyp_swap_bytes( void *data, NV_INT32 size, NV_INT32 num, NV_INT32 endian )
{
    static NV_U_CHAR buf[64];
    NV_INT32   i, j;
    NV_INT32   index1, index2;
    NV_U_CHAR *cdata;

    /* Check if swap is required */
    if ( (endian == HYP_FTYPE_BIGENDIAN && hyp_endian) || (endian == HYP_FTYPE_LITENDIAN && !hyp_endian) ) return;


    /* Can't byte swap one byte types! */
    if ( size == 1 )
        return;

    /* Swap the data */
    cdata = (NV_U_CHAR*)data;
    for ( i = 0; i < num; i++ )
    {
        index1 = i*size;
        index2 = (i+1)*size - 1;

        for ( j = 0; j < size; j++ )
        {
            buf[j] = cdata[index2-j];
        }

        for ( j = 0; j < size; j++ )
        {
            cdata[index1+j] = buf[j];
        }
    }
}

/*! <pre>
 *******************************************************************************
 * Function  : hyp_free_node_data
 * Arguments :
 * Returns   :
 * Purpose   : Frees any data in a node.
 *******************************************************************************
 </pre>*/
static void         hyp_free_node_data( HypNode *node )
{
    if( node->hypos )
        free( node->hypos );
}

/***************************************************************************/
/*!

  - Module Name:        hyp_init_hypothesis

  - Programmer(s):      IVS; modified by J. Parker (SAIC)

  - Date Written:       April 2006

  - Purpose:            Opens the hyp   (huge) file.

  - Arguments:
                        - hnd             =   PFM file handle
                        - path            =   Pathname of file

  - Return Value:
                        - SUCCESS
                        - OPEN_HYP_OPEN_ERROR

****************************************************************************/

NV_INT32 hyp_init_hypothesis (int hnd, HypHead *p, int new, BIN_HEADER *bin_header)
{

#ifdef PFM_DEBUG
    fprintf (stderr,"%s %d\n",__FILE__,__LINE__);
#endif

    char      *data;
    NV_INT32   i, j, percent, old_percent;


    if (new) {
      p->version            = HYP_VERSION;
      p->ncols              = bin_header->bin_width;
      p->nrows              = bin_header->bin_height;
      p->readonly           = NVFalse;
      p->endian             = hyp_endian;
      p->deleted_ptr        = 0;
      p->file_size          = HYP_ASCII_HEADER_SIZE + HYP_BINARY_HEADER_SIZE + ((NV_INT64)p->ncols * (NV_INT64)p->nrows * HYP_NODE_RECORD_SIZE);
      p->bin_header         = *bin_header;
      /* Write the header information to the file */
      hyp_write_header( p );

      /* Write empty information for the HypNodes */
      PFM_FSEEK( p->filehnd, HYP_ASCII_HEADER_SIZE + HYP_BINARY_HEADER_SIZE, SEEK_SET );
      data        = (char*)malloc( HYP_NODE_RECORD_SIZE );
      memset( data, 0, HYP_NODE_RECORD_SIZE );
      old_percent = 0;
      for( i = 0; i < p->nrows; i++ )
      {
          /* Write row data */
          for( j = 0; j < p->ncols; j++ )
          {
              PFM_FWRITE( (void*)data, HYP_NODE_RECORD_SIZE, 1, p->filehnd );
          }

          /* Update status callback */
          percent = (NV_INT32)( ((NV_FLOAT32)i / p->nrows) * 100.0 );
          if (percent != old_percent)
          {
              printf ("Creating hypothesis file (%d%c)\n", percent, 37);
              fflush(stdout);
              old_percent = percent;
          }
      }
      free( data );
    }  /* if creating new file */
    else {  /* existing file */
      /* Read the header information from the file */
      hyp_read_header( p );

      /* Check that hyp version is proper */
      if( p->version < 1.0 || p->version > HYP_VERSION )
      {
        sprintf( hyp_err_str, "Hypothesis file had unsupported version '%.2f'. This file may have been created with a newer version of the software.", p->version );
        return 0;
      }

      /* Check that hyp size is proper */
      if( (p->ncols != bin_header->bin_width) ||
          (p->nrows != bin_header->bin_height) )
      {
        sprintf( hyp_err_str, "Hypothesis file did not match PFM file in size." );
          return 0;
      }
    }
    return (pfm_error = SUCCESS);
}


