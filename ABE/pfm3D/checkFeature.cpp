#include "pfm3D.hpp"

NV_BOOL checkFeature (MISC *misc, QString feature_search_string, NV_INT32 ftr, NV_BOOL *highlight, QString *feature_info)
{
  NV_BOOL pass = NVTrue;


  *highlight = NVFalse;
  *feature_info = "";


  QString string0 (misc->feature[ftr].description);

  QString string1 (misc->feature[ftr].remarks);

  if (string1.isEmpty ())
    {
      if (!string0.isEmpty ()) *feature_info = string0;
    }
  else
    {
      if (string0.isEmpty ())
        {
          *feature_info = string1;
        }
      else
        {
          *feature_info = string0 + " :: " + string1;
        }
    }

  QString low_string = feature_info->toLower ();


  //  If we're doing a feature search we have a few things to check to see if we're going to display the feature.

  if (!feature_search_string.isEmpty ())
    {
      //  Check for the search string.

      if (misc->abe_share->feature_search_invert)
        {
          if (low_string.contains (feature_search_string.toLower ()))
            {
              if (misc->abe_share->feature_search_type) pass = NVFalse;
            }
          else
            {
              *highlight = NVTrue;
            }
        }
      else
        {
          if (!low_string.contains (feature_search_string.toLower ()))
            {
              if (misc->abe_share->feature_search_type) pass = NVFalse;
            }
          else
            {
              *highlight = NVTrue;
            }
        }
    }

  return (pass);
}
