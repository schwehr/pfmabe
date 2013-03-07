
/*********************************************************************************************

    This program is public domain software that was developed by the U.S. Naval Oceanographic
    Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.

*********************************************************************************************/


#ifndef VERSION

#define     VERSION     "PFM Software - pfm3D V2.58 - 06/30/11"

#endif

/*


    Version 1.0
    Jan C. Depner
    09/10/08

    First working version.


    Version 1.01
    Jan C. Depner
    10/09/08

    Changed rotation code so that control is in both the Y and ZX directions.


    Version 1.02
    Jan C. Depner
    10/20/08

    Moved getNearestPoint.cpp into nvMapGL.cpp


    Version 2.00
    Jan C. Depner
    10/22/08

    Using quaternions to handle rotations (don't ask).  Quaternion code from Superliminal Software:
    http://www.superliminal.com/sources/sources.htm
    Apparently this is by Melinda Green but it is an "Implementation of a simple C++ quaternion
    class called "Squat". Popularized by a seminal paper by Ken Shoemake, a quaternion represents a
    rotation about an axis.  Squats can be concatenated together via the * and *= operators and
    converted back and forth between transformation matrices. Implementation also includes a wonderful
    3D vector macro library by Don Hatch."  Which it didn't ;-)  I had to go find the vec.h code
    online.  Made a couple of minor modifications to squat.cpp. 


    Version 2.01
    Jan C. Depner
    10/28/08

    More intuitive handling of the Y rotations (Z in our world).


    Version 2.02
    Jan C. Depner
    11/21/08

    Using setTargetPoints instead of drawSphere to display targets.  This will allow us in future to edit the
    targets.  Made controls match pfmEdit3D.


    Version 2.03
    Jan C. Depner
    11/26/08

    Fix rotation bug when near flat view.


    Version 2.04
    Jan C. Depner
    12/08/08

    Flush map after turning targets off.


    Version 2.10
    Jan C. Depner
    01/23/09

    Changed to XOR for movable objects because redrawing the point cloud every time made the response too
    slow when you were viewing a lot of points.  I saved the original, non-XOR version in nvMapGL.cpp.noXOR.


    Version 2.20
    Jan C. Depner
    02/11/09

    Added subsampled layer for rotation and zooming.  Changed wheel zoom to be continuous until stopped.
    Added ability to edit from this window (actually tells pfmView to do a polygon edit).  Added coordination
    between pfmView, pfmEdit3D, and pfm3D.  When pfmEdit3D is started pfm3D will unload it's display lists
    and memory unless you move the cursor back into pfm3D in which case it will reload until you leave again.
    Lists and memory will be reloaded upon exit from pfmEdit3D.  Fixed problems with rubberband polygons.


    Version 2.21
    Jan C. Depner
    02/12/09

    Added draw scale option to prefs.  It's not really a scale, just a box at the moment.  Also, added ability
    to change background color.


    Version 2.22
    Jan C. Depner
    02/18/09

    Numerous hacks to try to fix the Windoze version of nvMapGL.cpp.  Added the extended help feature.


    Version 2.23
    Jan C. Depner
    02/22/09

    Hopefully handling the movable objects (XORed) correctly now.  This should make the Windoze version
    a bit nicer.


    Version 2.24
    Jan C. Depner
    02/24/09

    Finally figured it out - I have to handle all of the GL_BACK and GL_FRONT buffer swapping manually.  If
    I let it auto swap it gets out of control.


    Version 2.25
    Jan C. Depner
    03/24/09

    Added GUI control help.  Stop Mouse wheel zoom on reversal of direction.


    Version 2.26
    Jan C. Depner
    03/25/09

    Cleaned up the mouse wheel zoom.  Fixed the flicker at the end of a rotate or zoom operation.


    Version 2.30
    Jan C. Depner
    04/02/09

    Replaced support for NAVO standard target (XML) files with support for Binary Feature Data (BFD) files.


    Version 2.31
    Jan C. Depner
    04/14/09

    Added selectable feature marker size option.


    Version 2.32
    Jan C. Depner
    04/23/09

    Changed the acknowledgements help to include Qt and a couple of others.


    Version 2.33
    Jan C. Depner
    04/27/09

    Replaced QColorDialog::getRgba with QColorDialog::getColor.


    Version 2.34
    Jan C. Depner
    05/04/09

    Use paintEvent for "expose" events in nvMapGL.cpp.  This wasn't a problem under compiz but shows up
    under normal window managers.


    Version 2.40
    Jan C. Depner
    05/21/09

    Fixed the geotiff overlay problem caused by Z exaggeration.  Also, removed GeoTIFF (GDAL) specific code from
    nvMapGL.cpp and moved nvMapGL.cpp and .hpp to utility library.


    Version 2.41
    Jan C. Depner
    06/24/09

    Changed the BFD names to avoid collision with GNU Binary File Descriptor library.


    Version 2.42
    Jan C. Depner
    07/28/09

    Changed use of _sleep to Sleep on MinGW (Windows).


    Version 2.43
    Jan C. Depner
    09/11/09

    Fixed getColor calls so that cancel works.  Never forget!


    Version 2.44
    Jan C. Depner
    12/09/09

    Fixed VirtualBox close bug.


    Version 2.50
    Jan C. Depner
    01/25/10

    Added ability to diplay the On-The-Fly (OTF) grid from pfmView.  BTW - New Orleans Saints,
    2009 NFC Champions... Who Dat!!!


    Version 2.51
    Jan C. Depner
    04/29/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.  BTW - New Orleans Saints,
    2009 NFL Champions... Who Dat!!!


    Version 2.52
    Jan C. Depner
    08/17/10

    Replaced our kludgy old UTM transform with calls to the PROJ 4 library functions.  All hail the PROJ 4 developers!


    Version 2.53
    Jan C. Depner
    09/17/10

    Fixed bug displaying feature info.


    Version 2.54
    Jan C. Depner
    11/05/10

    Minor mods for auto scale changes to nvMapGL.cpp in nvutility library.


    Version 2.55
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 2.56
    Jan C. Depner
    01/15/11

    Added an exaggeration scrollbar to the left side of the window.


    Version 2.57
    Jan C. Depner
    04/15/11

    Fixed the geoTIFF reading by switching to using GDAL instead of Qt.  Hopefully Qt will get fixed eventually.


    Version 2.58
    Jan C. Depner
    06/30/11

    Added menu to allow displaying of all, unverified, verified, or no features (as opposed to just on/off).

*/
