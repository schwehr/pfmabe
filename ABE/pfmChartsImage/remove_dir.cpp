#include "pfmChartsImageDef.hpp"

void remove_dir (QString dir)
{
  QDir dirs;
  dirs.cd (dir);

  dirs.setFilter (QDir::Dirs | QDir::Hidden);


  //  Get all files in this directory.

  QDir files;
  files.setFilter (QDir::Files | QDir::Hidden);

  if (files.cd (dir))
    {
      QFileInfoList flist = files.entryInfoList ();
      for (NV_INT32 i = 0 ; i < flist.size () ; i++) remove (flist.at (i).absoluteFilePath ().toAscii ());
    }


  //  Get all directories in this directory.

  QFileInfoList list = dirs.entryInfoList ();

  QStringList dirList;
  for (NV_INT32 i = 0 ; i < list.size () ; i++)
    {
      //  Note that we're using absoluteFilePath.  That's because the filter on dirs is QDir::Dirs.

      if (list.at (i).fileName () != "." && list.at (i).fileName () != "..") 
        dirList.append (list.at (i).absoluteFilePath ());
    }


  //  Get all subordinate directories.

  QStringList::Iterator sit = dirList.begin ();
  while (sit != dirList.end ())
    {
      if (dirs.cd (*sit))
        {
          QFileInfoList nlist = dirs.entryInfoList ();
          QFileInfoList::Iterator nfit = nlist.begin ();

          while (nfit != nlist.end ())
            {
              if (nfit->fileName () != "." && nfit->fileName () != "..") 
                dirList.append (nfit->absoluteFilePath ());
              ++nfit;
            }
        }
 
      ++sit;
    }


  //  Remove all files in all subordinate directories (reverse order since we're removing directories).

  for (NV_INT32 j = dirList.size () - 1 ; j >= 0 ; j--)
    {
      files.setFilter (QDir::Files | QDir::Hidden);

      if (files.cd (dirList.at (j)))
        {
          QFileInfoList flist2 = files.entryInfoList ();

          for (NV_INT32 i = 0 ; i < flist2.size () ; i++) remove (flist2.at (i).absoluteFilePath ().toAscii ());
        }


      //  Remove each directory

      dirs.rmpath (dirList.at (j));
    }


  //  Remove the top level directory

  dirs.rmpath (dir);
}
