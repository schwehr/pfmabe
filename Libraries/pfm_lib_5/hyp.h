
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
 * LIBRARY        : part of pfm_lib                    )                     *
 * AUTHOR(S)      : Graeme Sweet (modified by J. Parker - SAIC)              *
 * FILE           : hyp.h (cloned from pcube.h)                              *
 * DATE CREATED   : Jan 13th, 2004 (modified on May 2006)                    *
 * PURPOSE        :                                                          *
 *  This library provides an interface to the hypothesis file of a PFM.      *
 *  A list of generated hypotheses is stored in a directory with extension   *
 *  ".hyp".                                                                  *
 *                                                                           *
 *****************************************************************************
 </pre>*/
#ifndef HYP_H
#define HYP_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "pfm.h"
#include "pfm_nvtypes.h"

/*****************************************************************************
 ***
 *** Hypothesis Defines and Typedefs
 ***
 *****************************************************************************/

/*! Typedefs for progress callback */
typedef void (*HYP_PROGRESS_CALLBACK) (int state, int percent);

/*! Slash strings for unix and windows */
#ifdef NVWIN3X     /* Windows Operating System */
#define HYP_SLASH '\\'
#define HYP_SLASH_STR "\\"
#else            /* Unix operating system */
#define HYP_SLASH '/'
#define HYP_SLASH_STR "/"
#endif
/* PFM uses SEPARATOR in pfm_io.c  and HUGE_SLASH in huge_io.c */


/* Defines for the endian when writing binary data */
#ifdef IVSPC    /* Should key off of NV_WIN3X and NV_LINUX ? */
#define HYP_LITENDIAN
#else
#define HYP_BIGENDIAN
#endif

/* Endian file types when writing binary data */
#define HYP_FTYPE_BIGENDIAN 0
#define HYP_FTYPE_LITENDIAN 1

/* Constants used for the library */
#define HYP_VERSION                 1.02f
#define HYP_ASCII_HEADER_SIZE       512
#define HYP_BINARY_HEADER_SIZE      512
#define HYP_NODE_RECORD_SIZE        16
#define HYP_HYPOTHESIS_RECORD_SIZE  24
#define HYP_DELETED_POINTER_POS    (HYP_ASCII_HEADER_SIZE + 4)
#define HYP_FILE_SIZE_POS          (HYP_ASCII_HEADER_SIZE + 4 + 8)
#define HYP_POINTER_POS            (HYP_HYPOTHESIS_RECORD_SIZE - 8)
#define HYP_CONF_99PC               2.56f /*!< Scale for 99% CI on Unit Normal */
#define HYP_CONF_95PC               1.96f /*!< Scale for 95% CI on Unit Normal */
#define HYP_CONF_90PC               1.69f /*!< Scale for 90% CI on Unit Normal */
#define HYP_CONF_68PC               1.00f /*!< Scale for 68% CI on Unit Normal */

#ifndef __CUBE_PRIVATE_H__
typedef enum {
        CUBE_PRIOR = 0,
        CUBE_LHOOD,
        CUBE_POSTERIOR,
        CUBE_PREDSURF,
        CUBE_UNKN
} CubeExtractor;

#ifndef __STDTYPES_H__
#ifndef False
typedef enum {
        False = 0,
        True
} Bool;
#endif
#endif

  /*!  Combined Bathymetric Uncertainty Estimator (CUBE).  */

typedef struct _cube_param {
        Bool                   init;           /*!< System mapsheet initialisation marker */
        NV_FLOAT32             no_data_value;  /*!< Value used to indicate 'no data' (typ. FLT_MAX) */
        CubeExtractor          mthd;           /*!< Method used to extract information from sheet */
        NV_FLOAT64             null_depth;     /*!< Depth to initialise estimates */
        NV_FLOAT64             null_variance;  /*!< Variance for initialisation */
        NV_FLOAT64             dist_exp;       /*!< Exponent on distance for variance scale */
        NV_FLOAT64             inv_dist_exp;   /*!< 1.0/dist_exp for efficiency */
        NV_FLOAT64             dist_scale;     /*!< Normalisation coefficient for distance (m) */
        NV_FLOAT64             var_scale;      /*!< Variance scale dilution factor */
        NV_FLOAT64             iho_fixed;      /*!< Fixed portion of IHO error budget (m^2) */
        NV_FLOAT64             iho_pcent;      /*!< Variable portion of IHO error budget (unitless) */
        NV_U_INT32             median_length;  /*!< Length of median pre-filter sort queue */
        NV_FLOAT32             quotient_limit; /*!< Outlier quotient upper allowable limit */
        NV_FLOAT32             discount;       /*!< Discount factor for evolution noise variance */
        NV_FLOAT32             est_offset;     /*!< Threshold for significant offset from current
                                                    estimate to warrant an intervention */
        NV_FLOAT32             bayes_fac_t;    /*!< Bayes factor threshold for either a single
                                                    estimate, or the worst-case recent sequence to
                                                    warrant an intervention */
        NV_U_INT32             runlength_t;    /*!< Run-length threshold for worst-case recent
                                                    sequence to indicate a drift failure and hence
                                                    to warrant an intervention */
        NV_FLOAT32             min_context;    /*!< Minimum context search range for hypothesis
                                                    disambiguation algorithm */
        NV_FLOAT32             max_context;    /*!< Maximum context search range */
        NV_FLOAT32             sd2conf_scale;  /*!< Scale from Std. Dev. to CI */
        NV_FLOAT32             blunder_min;    /*!< Minimum depth difference from pred. depth to
                                                    consider a blunder (m) */
        NV_FLOAT32             blunder_pcent;  /*!< Percentage of predicted depth to be considered
                                                    a blunder, if more than the minimum (0<p<1, typ. 0.25). */
        NV_FLOAT32             blunder_scalar; /*!< Scale on initialisation surface std. dev. at a node
                                                    to allow before considering deep spikes to be blunders. */
        NV_FLOAT32             capture_dist_scale;     /*!< Scale on predicted or estimated depth for how far out
                                                            to accept data.  (unitless; typically 0.05 for
                                                            hydrography but can be greater for geological mapping
                                                            in flat areas with sparse data) */
} CubeParam /* , *Cube */;

typedef struct _queue {
        NV_FLOAT32     depth;
        NV_FLOAT32     var;
        NV_FLOAT32     avg_tpe;
} Queue;

typedef struct _depth_hypothesis
{
  NV_FLOAT64           cur_estimate;     /*!< Current depth mean estimate */
  NV_FLOAT64           cur_variance;     /*!< Current depth variance estimate */
  NV_FLOAT64           pred_estimate;    /*!< Current depth next-state mean prediction */
  NV_FLOAT64           pred_variance;    /*!< Current depth next-state variance pred. */
  NV_FLOAT64           cum_bayes_fac;    /*!< Cumulative Bayes factor for node monitoring */
  NV_U_INT16           seq_length;       /*!< Worst-case sequence length for monitoring */
  NV_U_INT16           hypothesis_num;   /*!< Index term for debuging */
  NV_U_INT32           num_samples;      /*!< Number of samples incorporated into this node */
  NV_FLOAT32           var_estimate;     /*!< Running estimate of variance of inputs (m^2) */
  NV_FLOAT32           avg_tpe;          /*!< Average TPE as variance */
  struct _depth_hypothesis *next; /*!< Singly Linked List */
} Hypothesis;

typedef struct _queue *pQueue;
typedef struct _depth_hypothesis *pHypothesis;

typedef struct _cube_node {
        pQueue          queue;          /*!< Queued points in pre-filter */
        NV_U_INT32      n_queued;       /*!< Number of elements in queue structure */
        pHypothesis     depth;          /*!< Depth hypotheses currently being tracked */
        pHypothesis     nominated;      /*!< A nominated hypothesis from the user */
        NV_FLOAT32      pred_depth;     /*!< Predicted depth, or NaN for 'no update', or
                                             INVALID_DATA for 'no information available' */
        NV_FLOAT32      pred_var;       /*!< Variance of predicted depth, only valid if the
                                             predicted depth is (as above), meter^2 */
        Bool            write;          /*!< Debug write out */
} CNode /*, *CubeNode */;
#endif


/*****************************************************************************
 ***
 *** Hypothesis Structures
 ***
 *****************************************************************************/

/*! Structure for a hypothesis - each node may have many */
typedef struct _PHypothesis
{
    NV_FLOAT32  z;          /*!< Height associated with the hypothesis (m) */
    NV_FLOAT32  var;        /*!< Current variance for the node */
    NV_INT32    n_snds;     /*!< Number of soundings associated with the hypothesis */

    NV_FLOAT32  ci;         /*!< Confidence interval I associated with hypothesis (m) - calculated from var
                                 --- *which* CI depends on the parameter file, by default, 95% */
    NV_FLOAT32  avg_tpe;    /*!< Average vertical TPE for soundings in hypothesis */
}
PHypothesis;

/*! Structure for each node in the sheet - each one correcponds to a bin */
typedef struct _HypNode
{
    /* Variables stored in the file */
    NV_FLOAT32    best_hypo;        /*!< Best hypothesis choosen by the algorithm */
    NV_FLOAT32    best_unct;        /*!< Uncertainty of best hypothesis */
    CNode        *node;             /*!< Pointer to a CubeNode structure - used in cube_node.c */
    BIN_RECORD    bin;              /*!< Store the PFM bin information associated with this node */

    /* Predicted surface - calculated when algorithm is PRED_SURF */
    NV_FLOAT32    pred_depth;       /*!< Predicted depth - calculate to be median of the depths */
    NV_FLOAT32    pred_var;         /*!< Predicted variance */

    /* Final array of hypotheses */
    NV_INT32      n_hypos;          /*!< Number of hypotheses in array */
    PHypothesis  *hypos;            /*!< Array of all hypotheses for the node */
}
HypNode;

/*! Main Hypothesis structure */
typedef struct _HypHead
{
    /* Main variables for the Hypothesis file */
    NV_FLOAT32  version;            /*!< Version of hyp file. */
    NV_INT32    pfmhnd;             /*!< Handle for the pfm file this hyp is associated with. */
    NV_INT32    ncols;              /*!< Number of rows and columns - need to make sure this lines */
    NV_INT32    nrows;              /*!<   up with the pfm. */
    NV_INT32    filehnd;            /*!< The huge_io file handle used for reading/writing. */
    NV_U_CHAR   readonly;           /*!< If true, file is opened in read-only mode. */
    NV_U_CHAR   endian;             /*!< 1 for little endian, 0 for big */
    NV_INT64    deleted_ptr;        /*!< Pointer to first deleted record on disk - records organized in a chain */
    NV_INT64    file_size;          /*!< Size of the file in bytes - make sure whole file exists */
    NV_INT32    warning;            /*!< Non-zero if a warning has occured */

    /* The following are indices to attributes stored within the PFM file. */
    NV_INT32    vert_error_attr;    /*!< Vertical error attribute. */
    NV_INT32    horiz_error_attr;   /*!< Horizontal error attribute. */
    NV_INT32    num_hypos_attr;     /*!< Number of hypotheses attribute. */
    NV_INT32    hypo_strength_attr; /*!< Hypothesis strength attribute. */
    NV_INT32    uncertainty_attr;   /*!< Hypothesis uncertainty attribute. */
    NV_INT32    custom_hypo_flag;   /*!< Flag for marking a sounding as a custom hypothesis */

    /* User controllable parameters for the cube algorithm */
    /*  use hyp_get_parameters and hyp_set_parameters to modify */
    NV_INT32    hypo_resolve_algo;    /*!< Which hypothesis resolution algorithm to use - see HYP_RESOLVE_ */
    NV_FLOAT32  node_capture_percent; /*!< Only soundings less than this percent distance of the depth can
                                           away from a node center can contribute to the node. */

    /* Variables from the pfm */
    BIN_HEADER  bin_header;         /*!< The bin header from the PFM */

    /*! Parameters for running the cube - from cube.c */
    CubeParam  *cube_param;
}
HypHead;

/*****************************************************************************
 ***
 *** Hypothesis Public API functions
 ***
 *****************************************************************************/

/*! Returns a pointer to a Cube Node at the given row and column position. The caller
 * is responsible for freeing the object using 'hyp_free_node'.
 */
HypNode*   hyp_get_node( HypHead *p, NV_INT32 x, NV_INT32 y );

/*! Frees a node returned by hyp_get_node.
 */
void         hyp_free_node( HypNode *node );

/*! Sets a progress callback used when creating or recubeing an area.
 */
void         hyp_register_progress_callback( HYP_PROGRESS_CALLBACK progressCB );

/*! Returns a string describing the last error.
 */
char*        hyp_get_last_error();

NV_INT32     hyp_init_hypothesis (int hnd, HypHead *hyp, int nw, BIN_HEADER *bin_header);

HypHead*     hyp_open_file( char *list_file_path, NV_INT32 pfmhnd );

HypHead*     hyp_create_file( char *list_file_path, NV_INT32 pfmhnd,
                NV_INT32 horizErrorAttr, NV_INT32 vertErrorAttr,
                NV_INT32 numHyposAttr, NV_INT32 hypoStrengthAttr,
                NV_INT32 uncertaintyAttr, NV_INT32 customHypoFlag );

void         hyp_close_file( HypHead *p );

NV_INT32     hyp_write_header( HypHead *p );
NV_INT32     hyp_read_header( HypHead *p );
NV_INT32     hyp_read_node( HypHead *p, NV_INT32 x, NV_INT32 y, HypNode *node );
NV_INT32     hyp_write_node( HypHead *p, NV_INT32 x, NV_INT32 y, HypNode *node );

#ifdef  __cplusplus
}
#endif


#endif
