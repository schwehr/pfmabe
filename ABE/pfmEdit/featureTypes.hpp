
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



#define ROCK_TYPES 35
#define OFFSHORE_TYPES 22
#define LIGHT_TYPES 47
#define LIDAR_TYPES 9


//!  These are the CHART 1 Rock, Wreck, and Obstruction Types.

QString rockType[ROCK_TYPES] =
  {
    editFeature::tr ("Rock (islet) which does not cover"),
    editFeature::tr ("Rock which covers and uncovers"),
    editFeature::tr ("Rock awash at the level of chart datum"),
    editFeature::tr ("Dangerous underwater rock of uncertain depth"),
    editFeature::tr ("Dangerous underwater rock of known depth"),
    editFeature::tr ("Non-dangerous rock, depth known"),
    editFeature::tr ("Coral reef which covers and uncovers"),
    editFeature::tr ("Coral reef awash at level of chart datum"),
    editFeature::tr ("Dangerous underwater coral reef of uncertain depth"),
    editFeature::tr ("Dangerous underwater coral reef of known depth"),
    editFeature::tr ("Non-dangerous coral reef, depth known"),
    editFeature::tr ("Breakers"),
    editFeature::tr ("Wreck, hull always dry"),
    editFeature::tr ("Wreck, covers and uncovers"),
    editFeature::tr ("Submerged wreck, depth known"),
    editFeature::tr ("Submerged wreck, depth unknown"),
    editFeature::tr ("Wreck showing any portion of hull or superstructure at level of chart datum"),
    editFeature::tr ("Wreck showing mast or masts above chart datum only"),
    editFeature::tr ("Wreck, least depth known by sounding only"),
    editFeature::tr ("Wreck, least depth known, swept by wire drag or diver"),
    editFeature::tr ("Dangerous wreck, depth unknown"),
    editFeature::tr ("Sunken wreck, not dangerous to surface navigation"),
    editFeature::tr ("Wreck, least depth unknown, but considered to have a safe clearance to the depth shown"),
    editFeature::tr ("Foul ground, non-dangerous to navigation but to be avoided by vessels anchoring, trawling, etc"),
    editFeature::tr ("Foul area, foul with rocks or wreckage, dangerous to navigation"),
    editFeature::tr ("Obstruction, depth unknown"),
    editFeature::tr ("Obstruction, least depth known"),
    editFeature::tr ("Obstruction, least depth known, swept by wire drag or diver"),
    editFeature::tr ("Stumps of posts or piles, all or part of the time submerged"),
    editFeature::tr ("Submerged pile, stake, snag, well, deadhead, or stump"),
    editFeature::tr ("Fishing stakes"),
    editFeature::tr ("Fish trap, fish weirs, tunny nets"),
    editFeature::tr ("Fish haven (artificial fishing reef)"),
    editFeature::tr ("Fish haven with minimum depth"),
    editFeature::tr ("Shellfish culivation (stakes visible)")
  };


//!  These are the CHART 1 Offshore Installation Types.

QString offshoreType[OFFSHORE_TYPES] =
  {
    editFeature::tr ("Production platform, platform, oil derrick"),
    editFeature::tr ("Flare stack (at sea)"),
    editFeature::tr ("Mooring tower, Articulated Loading Platform (ALP), Single Anchor Leg Mooring (SALM)"),
    editFeature::tr ("Observation/research platform"),
    editFeature::tr ("Disused platform"),
    editFeature::tr ("Artificial island"),
    editFeature::tr ("Oil or gas installation buoy, Catenary Anchor Leg Mooring (CALM), Single Buoy Mooring (SBM)"),
    editFeature::tr ("Moored storage tanker"),
    editFeature::tr ("Submerged production well"),
    editFeature::tr ("Suspended well, depth over wellhead unknown"),
    editFeature::tr ("Suspended well, with depth over wellhead"),
    editFeature::tr ("Wellhead with height above the bottom"),
    editFeature::tr ("Site of cleared platform"),
    editFeature::tr ("Above water wellhead"),
    editFeature::tr ("Submarine cable"),
    editFeature::tr ("Submarine power cable"),
    editFeature::tr ("Disused submarine cable"),
    editFeature::tr ("Oil, gas pipeline"),
    editFeature::tr ("Waterpipe, sewer, outfall pipe, intake pipe"),
    editFeature::tr ("Buried pipeline pipe"),
    editFeature::tr ("Potable water intake, diffuser, or crib"),
    editFeature::tr ("Disused pipeline/pipe")
  };


//!  These are the CHART 1 Light, Buoy, and Beacon Types.

QString lightType[LIGHT_TYPES] =
  {
    editFeature::tr ("Major light, minor light, light, lighthouse"),
    editFeature::tr ("Lighted offshore platform"),
    editFeature::tr ("Lighted beacon tower"),
    editFeature::tr ("Lighted beacon"),
    editFeature::tr ("Articulated light, buoyant beacon, resilient beacon"),
    editFeature::tr ("Light vessel; Lightship, normally manned light vessel"),
    editFeature::tr ("Unmanned light vessel, light float"),
    editFeature::tr ("LANBY, superbuoy as navigational aid"),
    editFeature::tr ("Beacon"),
    editFeature::tr ("Buoy"),
    editFeature::tr ("Conical buoy, nun buoy"),
    editFeature::tr ("Can or cylindrical buoy"),
    editFeature::tr ("Spherical buoy"),
    editFeature::tr ("Pillar buoy"),
    editFeature::tr ("Spar buoy, spindle buoy"),
    editFeature::tr ("Barrel buoy"),
    editFeature::tr ("Super buoy"),
    editFeature::tr ("Mooring buoy"),
    editFeature::tr ("Lighted mooring buoy"),
    editFeature::tr ("Trot, mooring buoys with ground tackle"),
    editFeature::tr ("Firing danger area (Danger Zone) buoy"),
    editFeature::tr ("Target buoy"),
    editFeature::tr ("Marker ship"),
    editFeature::tr ("Barge"),
    editFeature::tr ("Degaussing range buoy"),
    editFeature::tr ("Cable buoy"),
    editFeature::tr ("Spoil ground buoy"),
    editFeature::tr ("Buoy marking outfall"),
    editFeature::tr ("ODAS buoy (Ocean Data Acquisition System), data collecting buoy of super buoy size"),
    editFeature::tr ("Wave recorder, current meter"),
    editFeature::tr ("Seaplane anchorage buoy"),
    editFeature::tr ("Buoy marking traffic separation scheme"),
    editFeature::tr ("Buoy marking recreation zone"),
    editFeature::tr ("Buoy privately maintained"),
    editFeature::tr ("Seasonal buoy"),
    editFeature::tr ("Beacon on submerged rock"),
    editFeature::tr ("Stake, pole"),
    editFeature::tr ("Perch, stake"),
    editFeature::tr ("Withy"),
    editFeature::tr ("Cairn"),
    editFeature::tr ("Beacon tower"),
    editFeature::tr ("Lattice tower"),
    editFeature::tr ("Leading beacon"),
    editFeature::tr ("Cable landing beacon"),
    editFeature::tr ("Refuge beacon"),
    editFeature::tr ("Firing danger area beacon"),
    editFeature::tr ("Notice board")
  };


//!  These are the special LiDAR category types.

QString lidarType[LIDAR_TYPES] =
  {
    editFeature::tr ("LIDAR survey, no data, too deep."),
    editFeature::tr ("LIDAR survey, no data, too shallow"),
    editFeature::tr ("LIDAR survey, no data, too turbid"),
    editFeature::tr ("LIDAR survey, no data, too calm (no surface return)"),
    editFeature::tr ("LIDAR survey, no data, tannin in water"),
    editFeature::tr ("LIDAR survey, no data, sunglint"),
    editFeature::tr ("LIDAR survey, shoreline depth swap"),
    editFeature::tr ("Informational, data does not meet IHO order 1"),
    editFeature::tr ("Informational, data does not meet IHO order 2")
  };
