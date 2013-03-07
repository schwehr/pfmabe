#include "geoSwath3D.hpp"

void actionKey (geoSwath3D *parent, nvMapGL *map __attribute__ ((unused)), POINT_DATA *data __attribute__ ((unused)),
                OPTIONS *options __attribute__ ((unused)), MISC *misc, QString key, NV_INT32 lock_point __attribute__ ((unused)))
{
  misc->abeShare->lock ();


  //  We set the modcode to 0 (i.e. nothing has changed)

  misc->abe_share->modcode = 0;


  //  We then set the action key for the external program (it should be monitoring
  //  misc->abe_share->key for it to change).

  misc->abe_share->key = (NV_U_INT32) key.toAscii()[0];


  misc->abeShare->unlock ();


  //  Then we wait at least one second for something to be done in the external program

  NV_BOOL hit = NVFalse;
  QTime t;
  t.start();
  do
    {
      //  The external program should set modcode to it's data type to signify that it has 
      //  done something

      if (misc->abe_share->modcode)
        {
          if (misc->abe_share->modcode == NO_ACTION_REQUIRED) break;


          //  After the external program sets modcode we want to lock shared memory.

          misc->abeShare->lock ();


          //  Unfortunately this is data type specific, which I really hate to put in here,
          //  but we have no other choice that I can see.

          switch (misc->abe_share->modcode)
            {
            default:
              break;
            }

          misc->abe_share->key = 0;


          //  Unlock it after we have processed the data.

          misc->abeShare->unlock ();


          break;
        }
    } while (t.elapsed () < 1000);


  if (hit) 
    {
      parent->redrawMap (NVFalse);
    }
}
