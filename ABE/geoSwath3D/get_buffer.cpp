#include "geoSwath3D.hpp"


/***************************************************************************\
*                                                                           *
*   Module Name:        get_buffer                                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       May 1999                                            *
*                                                                           *
*   Purpose:            Get the rectangle of data from the file.            *
*                                                                           *
*   Return Value:       None                                                *
*                                                                           *
\***************************************************************************/

void get_buffer (POINT_DATA *data, MISC *misc, NV_INT32 record)
{
  NV_INT32        count;


  NV_INT32 get_bounds (POINT_DATA *data, MISC *misc);


  misc->min_z = 99999999.0;
  misc->max_z = -99999999.0;


  //  Flush all modified points in the last buffer.
    
  misc->statusProgLabel->setText (geoSwath3D::tr ("Writing..."));
  misc->statusProgLabel->setVisible (TRUE);
  misc->statusProg->setRange (0, data->count);
  misc->statusProg->setTextVisible (TRUE);
  qApp->processEvents();

  for (NV_INT32 i = 0 ; i < data->count ; i++)
    {
      if (data->oval[i] != data->val[i])
        {
          misc->statusProg->setValue (i);

          io_data_write (data->val[i], data->rec[i], data->sub[i], data->time[i]);
	}
    }

  misc->statusProg->reset ();
  misc->statusProg->setRange (0, 100);
  misc->statusProg->setValue (0);
  misc->statusProgLabel->setVisible (FALSE);
  misc->statusProg->setTextVisible (FALSE);
  qApp->processEvents ();


  //  Set the record number.

  set_record (record);


  //  Read the buffer.

  misc->statusProgLabel->setText (geoSwath3D::tr ("Reading..."));
  misc->statusProgLabel->setVisible (TRUE);
  misc->statusProg->setTextVisible (TRUE);
  qApp->processEvents();

  io_data_read (misc->num_records, data, misc->statusProg);

  misc->statusProg->reset ();
  misc->statusProg->setRange (0, 100);
  misc->statusProg->setValue (0);
  misc->statusProgLabel->setVisible (FALSE);
  misc->statusProg->setTextVisible (FALSE);
  qApp->processEvents ();


  //  Compute the total area bounds.

  count = get_bounds (data, misc);


  //  Set the 'modified' records.

  for (NV_INT32 i = 0 ; i < data->count ; i++) data->oval[i] = data->val[i];
}
