
/*********************************************************************************************

    This library is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

/*

    Waveform LIDAR Format (WLF)

    Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

    Date:        02/25/09



    The WLF data type is a simple binary format for archive/transfer of LIDAR data that
    may or may not have associated waveforms.  The following documentation describes the WLF
    API as well as the actual file format.  Other than the header (which is ASCII) the format
    will be incomprehensible gibberish if you try to look at it with a hex editor.  That's
    because all of the data is bit-packed and the waveforms are delta-code compressed.

    In the following code you will see some strange data types that start with NV_.  These data
    types (like NV_U_INT16) are a system independent means of assuring that we are using 8, 16, 32,
    and 64 bit signed or unsigned values.  They are all defined in the wlf_nvtypes.h header file.

    One point to keep firmly in mind.  The WLF format and API is not meant to be all things to all
    people.  I am trying to adhere very strongly to the KISS principle of development - Keep It
    Simple Stupid.  Many of the record fields in this format are shamelessly borrowed (read stolen)
    from the LAS 1.2 spec since it is a nice, simple LIDAR format.  Unfortunately, the LAS committee
    has not been interested in storing waveforms so we had to do something ;-)




    1) HEADER

    Over the years I have found that it is really nice if you can get an idea of what is in a
    particular file without having to resort to running a special program.  To this end I have
    used tagged ASCII headers for this format so that you can dump the header and get some
    information about the file by just using the "type /page" command (in an MS Command prompt
    window) or the "more" or "less" commands on Linux/UNIX/Mac OS/X.  This is a version 1.0
    ASCII header example:

        [VERSION] = PFM Software - WLF library V1.0 - 02/25/09
        [CREATION YEAR] = 2009
        [CREATION MONTH] = 03
        [CREATION DAY] = 03
        [CREATION DAY OF YEAR] = 062
        [CREATION HOUR] = 16
        [CREATION MINUTE] = 28
        [CREATION SECOND] = 24.00
        [MODIFICATION YEAR] = 2009
        [MODIFICATION MONTH] = 03
        [MODIFICATION DAY] = 03
        [MODIFICATION DAY OF YEAR] = 062
        [MODIFICATION HOUR] = 16
        [MODIFICATION MINUTE] = 28
        [MODIFICATION SECOND] = 24.00
        [MODIFICATION SOFTWARE] = PFM Software - charts2wlf V1.00 - 02/27/09
        [SECURITY CLASSIFICATION] = RIDICULOUS
        {DISTRIBUTION =
        Destroy before reading.
        }
        {DECLASSIFICATION =
        When hell freezes over.
        }
        {SECURITY CLASSIFICATION JUSTIFICATION =
        Because I said so.
        }
        {DOWNGRADE =
        Yeah, right.
        }
        [SOURCE] = JALBTCX
        [SYSTEM] = System 2, Type 1, Rate 1000
        [OBSERVED MIN X] = 145.68754230585
        [OBSERVED MIN Y] = 15.21468125740
        [OBSERVED MIN Z] = -25.389158249
        [OBSERVED MAX X] = 145.70073575313
        [OBSERVED MAX Y] = 15.22998887242
        [OBSERVED MAX Z] = 3.810504913
        [OBSERVED VALID MIN X] = 145.68754230585
        [OBSERVED VALID MIN Y] = 15.21468125740
        [OBSERVED VALID MIN Z] = -25.389158249
        [OBSERVED VALID MAX X] = 145.70073575313
        [OBSERVED VALID MAX Y] = 15.22998887242
        [OBSERVED VALID MAX Z] = 3.810504913
        [NUMBER OF RECORDS] = 33287
        [PROJECT] = NM_Saipan
        [MISSION] = 02MD06005_004_060320_2225
        [DATASET] = 02DS06005_004_060320_2225_A
        [FLIGHT ID] = Line 7-2
        [FLIGHT START YEAR] = 2006
        [FLIGHT START MONTH] = 03
        [FLIGHT START DAY] = 20
        [FLIGHT START DAY OF YEAR] = 079
        [FLIGHT START HOUR] = 22
        [FLIGHT START MINUTE] = 58
        [FLIGHT START SECOND] = 37.01
        [FLIGHT END YEAR] = 2006
        [FLIGHT END MONTH] = 03
        [FLIGHT END DAY] = 20
        [FLIGHT END DAY OF YEAR] = 079
        [FLIGHT END HOUR] = 22
        [FLIGHT END MINUTE] = 59
        [FLIGHT END SECOND] = 10.41
        [WLF USER FLAG 01 NAME] = Shallow processed
        [WLF USER FLAG 02 NAME] = Shoreline depth swapped
        [WLF USER FLAG 03 NAME] = Land
        [WLF USER FLAG 04 NAME] = Second depth present
        [ATTRIBUTE 01 NAME] = Abbreviated Depth Confidence
        [MIN ATTRIBUTE 01] = -100.0
        [MAX ATTRIBUTE 01] = 100.0
        [ATTRIBUTE 01 SCALE] = 1.0
        [ATTRIBUTE 02 NAME] = Full Depth Confidence
        [MIN ATTRIBUTE 02] = 0.0
        [MAX ATTRIBUTE 02] = 1000000.0
        [ATTRIBUTE 02 SCALE] = 1.0
        {COMMENTS = 
        NOTE: No warranties, either express or implied, are hereby given. All software is supplied as is,
        without guarantee.  The user assumes all responsibility for damages resulting from the use of these
        features, including, but not limited to, frustration, disgust, system abends, disk head-crashes,
        general malfeasance, floods, fires, shark attack, nerve gas, locust infestation, cyclones,
        hurricanes, tsunamis, local electromagnetic disruptions, hydraulic brake system failure, invasion,
        hashing collisions, normal wear and tear of friction surfaces, cosmic radiation, inadvertent
        destruction of sensitive electronic components, windstorms, the Riders of Nazgul, infuriated chickens,
        malfunctioning mechanical or electrical sexual devices, premature activation of the distant early
        warning system, peasant uprisings, halitosis, artillery bombardment, explosions, cave-ins, and/or
        frogs falling from the sky.
        }
        [Z UNITS] = METERS
        [NULL Z VALUE] = -998.00000
        [MAX NUMBER OF RETURNS PER RECORD] = 1
        {WELL-KNOWN TEXT =
        COMPD_CS["WGS84 with ellipsoid Z",GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,
        AUTHORITY["EPSG","7030"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,
        AUTHORITY["EPSG","8901"]],UNIT["degree",0.01745329251994328,AUTHORITY["EPSG","9108"]],AXIS["Lat",NORTH],
        AXIS["Long",EAST],AUTHORITY["EPSG","4326"]],VERT_CS["ellipsoid Z in meters",VERT_DATUM["Ellipsoid",2002],
        UNIT["metre",1],AXIS["Z",UP]]]
        }
        [MIN X] = 145.68753660000
        [MAX X] = 145.70073575000
        [X SCALE] = 100000000.00000000000
        [MIN Y] = 15.21467045000
        [MAX Y] = 15.22998887000
        [Y SCALE] = 100000000.00000000000
        [MIN Z] = -8388.000000000
        [MAX Z] = 8388.000000000
        [SENSOR Z SCALE] = 1000.000000000
        [SENSOR MIN X] = 145.68753660000
        [SENSOR MAX X] = 145.70073575000
        [SENSOR X SCALE] = 100000000.00000000000
        [SENSOR MIN Y] = 15.21467045000
        [SENSOR MAX Y] = 15.22998887000
        [SENSOR Y SCALE] = 100000000.00000000000
        [SENSOR MIN Z] = -8388.000000000
        [SENSOR MAX Z] = 8388.000000000
        [SENSOR Z SCALE] = 1000.000000000
        [SENSOR MIN ROLL] = -30.000000000
        [SENSOR MAX ROLL] = 30.000000000
        [SENSOR ROLL SCALE] = 10.000000000
        [SENSOR MIN PITCH] = -30.000000000
        [SENSOR MAX PITCH] = 30.000000000
        [SENSOR PITCH SCALE] = 10.000000000
        [SENSOR MIN HEADING] = 0.000000000
        [SENSOR MAX HEADING] = 360.000000000
        [SENSOR HEADING SCALE] = 10.000000000
        [MIN SCAN ANGLE] = -90.000000000
        [MAX SCAN ANGLE] = 90.000000000
        [SCAN ANGLE SCALE] = 10.000000000
        [MIN NADIR ANGLE] = 0.000000000
        [MAX NADIR ANGLE] = 30.000000000
        [NADIR ANGLE SCALE] = 10.000000000
        [MIN WATER SURFACE] = -8388.000000000
        [MAX WATER SURFACE] = 8388.000000000
        [WATER SURFACE SCALE] = 1000.000000000
        [MIN Z OFFSET] = -250.000000000
        [MAX Z OFFSET] = 250.000000000
        [Z OFFSET SCALE] = 1000.000000000
        [Z OFFSET DATUM] = Local MLLW defined by tide gage data
        [MIN HORIZONTAL UNCERTAINTY] = 0.000000000
        [MAX HORIZONTAL UNCERTAINTY] = 10.000000000
        [HORIZONTAL UNCERTAINTY SCALE] = 1000.000000000
        [MIN VERTICAL UNCERTAINTY] = 0.000000000
        [MAX VERTICAL UNCERTAINTY] = 10.000000000
        [VERTICAL UNCERTAINTY SCALE] = 1000.000000000
        [NUMBER OF WAVEFORMS] = 4
        [WAVEFORM 01 NAME] = PMT
        [MIN WAVEFORM 01] = 0
        [MAX WAVEFORM 01] = 255
        [WAVEFORM 01 COUNT] = 501
        [WAVEFORM 02 NAME] = APD
        [MIN WAVEFORM 02] = 0
        [MAX WAVEFORM 02] = 255
        [WAVEFORM 02 COUNT] = 201
        [WAVEFORM 03 NAME] = IR
        [MIN WAVEFORM 03] = 0
        [MAX WAVEFORM 03] = 255
        [WAVEFORM 03 COUNT] = 201
        [WAVEFORM 04 NAME] = Raman
        [MIN WAVEFORM 04] = 0
        [MAX WAVEFORM 04] = 255
        [WAVEFORM 04 COUNT] = 81
        [POSIX TIME SECOND BITS] = 32
        [POSIX TIME NANOSECOND BITS] = 30
        [CLASSIFICATION BITS] = 12
        [STATUS BITS] = 11
        [WAVEFORM ADDRESS BITS] = 36
        [WAVEFORM BLOCKSIZE BITS] = 24
        [WAVEFORM BLOCK OFFSET] = 1130720
        [HEADER SIZE] = 65536
        [RECORD SIZE] = 32
        [END OF HEADER]


    The first question asked is usually "Why not use XML".  The answer is that, in most operating
    systems, the file browser will try to determine the type of file by either the extension or,
    if that fails, by reading a small bit of the file.  If I were to use XML then most file
    browsers would try to open the, possibly multi-gigabyte, file with Firefox or Internet Exploder
    (no, that wasn't a typo).  The main reason that you would use XML is so that many different
    browsers or applications would be able to read the file.  That will not be an issue here since
    the API is already in place.  If you try to use some other API to read these files - CAVEAT EMPTOR.

    Most of the fields in the header are pretty obvious but I'll try to address the ones that may be a
    bit obscure and also point out mandatory fields.  All fields are separated by either a line feed
    (on Linux/UNIX) or a carriage return and line feed (on Windows).  This is handled nicely by C.
    Fields that begin with [ are single line fields.   Fields that begin with { are multi-line fields
    and will continue until a } is encountered in the first column of a line.

    The [VERSION] field is mandatory in order to maintain backward compatibility as we add features in the
    future.  This field will be added whenever the file is created.  The [HEADER SIZE] field is mandatory
    and can be any value.  It really just tells you where the header ends and the data begins.  It will
    be set to 65536 (for now).  All dates and times will be UTC (I don't even want to think about local time
    and/or daylight savings time).  The [SECURITY CLASSIFICATION], [DISTRIBUTION], [SECURITY CLASSIFICATION
    JUSTIFICATION], [DECLASSIFICATION], and [DOWNGRADE] fields are optional (the Navy wants that information
    in all of their data files).

    The [WLF USER FLAG ??] fields are flag bits that can be set in the wlf_record.status field.  There may be up
    to 5 of these defined by the application.

    The [ATTRIBUTE ?? NAME] field will allow the application to define up to 10 optional attributes for special 
    cases.  If the [ATTRIBUTE ?? SCALE] field is not defined the attribute will not be stored.  If it is defined,
    the [MIN ATTRIBUTE ??], [MAX ATTRIBUTE ??], and [ATTRIBUTE ?? NAME] must be defined as well.  Even though
    it is possible to define [ATTRIBUTE 07 NAME] without defining the previous six attributes please don't ;-)

    All fields from [Z UNITS] on are required to be set in the header structure prior to creation of the file.
    After the file has been created these will be returned when you open the file but they will not be modifiable.

    [Z UNITS] is mandatory and will be either WLF_METERS, WLF_FEET, or WLF_FATHOMS.  If anyone wants to store
    things in other units we can add them.  One of my personal favorites is cubits (the Roman cubit not the
    Egyptian Royal cubit but I'm not averse to the Sumerian Nippur cubit).  And then of course there is the 
    ever popular Willett (2.4 inches).  The [NULL Z VALUE] field is mandatory.  Note that the NULL Z VALUE
    doesn't have to be in the range you set for [MIN Z] and [MAX Z].  We add 1.0 to the range on creation and
    store the null z values as [MAX Z] plus 1.0.  When you read a record the API will stuff the null value into
    the field if it is set to [MAX Z] plus 1.0.  Z values will always be stored as elevations (as opposed to depths).

    The [MIN ?], [MAX ?], and [? SCALE] fields will be used to determine how many bits we will be using to store
    the scaled, offset integer values.  For example, if the [MIN X] field is -180.0, the [MAX X] field is 180.0,
    and the [X SCALE] is 100000000.000000000 then we will require 36 bits to store the values.  This is easily
    computed using the following equation:

        bits = NINT64 (log10 ((wlf_header.max_x - wlf_header.min_x) * wlf_header.x_scale) / log10 (2.0) + 0.5);

    where NINT64 is a 64 bit integer rounding function.  If I just happen to know that the file I'm creating
    will have a minimum X value (longitude) of -80.0 and a maximum X value of -79.0 I could store the X values
    in 27 bits instead of 36.  If you don't want to include a particular field, like intensity, just set the
    [? SCALE] field for that field to 0.0.

    {WELL-KNOWN TEXT} is a required field.  This is the Well-Known Text coordinate and datum information that
    is defined in the Open GIS Coordinate Transformations specification.  These are supported by the GDAL/OGR
    library so you can use OGRSpatialReference::importFromWkt() method (or the corresponding C function) and
    then warp from one to another pretty easily).

    [NUMBER OF RECORDS] is mandatory as is [MAX NUMBER OF RETURNS PER RECORD], and [NUMBER OF WAVEFORMS].  If
    [NUMBER OF WAVEFORMS] is set to a value other than 0 then the [WAVEFORM N NAME], [MIN WAVEFORM N], and
    [MAX WAVEFORM N] fields are mandatory for each of the waveforms.  The assumption here is that waveform
    values are integer and smaller than 32 bits.  [NUMBER OF RECORDS] will be provided by the API on
    wlf_close_file after wlf_create_file.

    [END OF HEADER] is the last mandatory field.  It is placed in the file by the API.  All data from the
    [END OF HEADER] to the [HEADER SIZE] byte address will be SPACE (' ') filled.

    Please see the WLF_HEADER structure definition and footnotes in the code below to get more detailed
    information on each field of the header.




    2) POINT DATA RECORD

    So, if that's the header, let's get into the record data.  The data will be written as offset, scaled integers, 
    bit-packed in unsigned byte (i.e. character) arrays.  The advantage to using byte arrays is two-fold.  First,
    there are no ENDIAN issues.  There is no ordering when you write or read bytes.  Second, this allows us to read
    blocks of data instead of pieces of the structure.  The immediate reaction to that statement is usually "why
    not write structures".  The reason is that on a byte addressable system with a one-byte variable in a structure
    the system will write one byte at that point in the structure.  On a 32 bit word addressable system it will
    write 4 bytes at that location for the same one-byte variable.  So, due to this, you have to write each piece
    of the structure separately in order to maintain portability (or always use 4 byte boundaries/variables or use
    non-standard pragmas).  In addition, the structure wastes space in the file.  Why use two bytes to store a
    value that will fit into 12 bits?


    Given the above example header a WLF record would be stored internally as follows:


        tv_sec - 32 bits
        tv_nsec - 31 bits
        horizontal_uncertainty - 14 bits
        vertical_uncertainty - 14 bits
        x - 21 bits
        y - 21 bits
        z - 24 bits
        sensor_x - 21 bits
        sensor_y - 21 bits
        sensor_z - 24 bits
        waveform - 2 bits
        waveform_point - 8 bits
        sensor_roll - 10 bits
        sensor_pitch - 10 bits
        sensor_heading - 12 bits
        scan_angle - 11 bits
        nadir_angle - 9 bits
        water_surface - 24 bits
        z_offset - 19 bits
        number_of_returns - 0 bits
        return_number - 0 bits
        point source - 0 bits
        edge_of_flight_line - 0 bits
        intensity - 0 bits
        attribute 01 - 8 bits
        attribute 02 - 20 bits
        rgb - 0 bits
        reflectance - 0 bits
        classification - 12 bits
        status - 11 bits
        waveform_address - 36 bits

    The total is 415 bits or 52 bytes.  Some assumptions are made here - classification, status, and 
    waveform_address sizes (and thus allowable range) are fixed for each version of the WLF API.  If we
    need to extend these in future we'll increment the major version and make sure that the API is backward
    compatible.  An explanation of the classification numbering system can be found in wlf_class.c or 
    wlf_class.h.

    If, in the above instance, we had not needed the uncertainties and the water surface values the record
    size would have been 363 bits or 46 bytes.  Without waveforms it would be 327 bits or 41 (actually 40.875)
    bytes.  In the interest of simplicity, we round the byte count up instead of trying to combine multiple
    records to use the partial bits.  If we had tried to save the structure as it exists in memory we would
    have had to store at least 284 bytes (276 + 8 bytes for the waveform address).  So, it should be
    intuitively obvious to the most casual observer that the storage requirements are entirely dependent upon
    the fields that are stored and the min, max, and scale values, not on the size of the fields that are in
    the structure.




    3) WAVEFORM RECORD

    Each waveform array is delta coded (difference coding between records) and bit packed.  The waveforms are
    stored as follows:

        24 bits - size of block of delta coded waveform arrays (this may go to 32 bits in the future if we need more 
                  than 16,777,216 bytes of compressed waveform data)

        [waveform 0]
        X bits - signed starting value (X is either 8, 16, or 32 depending upon the range of the waveform data)
        X bits - signed bias value
        5 bits - number of bits used to store delta coding offsets (this is NB for this waveform)
        NB bits - first offset - bias
        NB bits - offset from first - bias
        NB bits - offset from second - bias
        .
        .
        .
        NB bits - last offset - bias (wlf_header.waveform_count[0] - 1)

        [waveform 1]
        X bits - signed starting value (X is either 8, 16, or 32 depending upon the range of the waveform data)
        X bits - signed bias value
        5 bits - number of bits used to store delta coding offsets (this is NB for this waveform)
        NB bits - first offset - bias
        NB bits - offset from first - bias
        NB bits - offset from second - bias
        .
        .
        .
        NB bits - last offset - bias (wlf_header.waveform_count[1] - 1)
        .
        .
        .
        etc
        etc
        etc up to wlf_header.number_of_waveforms


    The waveforms are stored at the end of the file and use the “waveform_address” field of the stored point record
    (this is not in the WLF_RECORD structure) to get to each block of waveforms.  Please note that the waveform_address
    is actually added to the [WAVEFORM BLOCK OFFSET] value to get the actual address of the associated waveforms.
    Of course you may not have, or want, to save the waveforms so that would make things a lot easier.  The amount of
    compression will depend on the amount of dead space in the digitized waveforms.  If you are using a 10 bit digitizer
    you should get quite a bit of savings since most systems will store that in 16 bits.  The Optech CHARTS system
    uses an 8 bit digitizer so the compression is not as good.  In practice we're seeing about a 36 percent reduction
    in file size.




    4) PUBLIC API FUNCTIONS

    In keeping with the KISS principle, their are only 14 public I/O functions for the WLF API.  For more detailed 
    information on each function please see the Public API I/O function declarations in the code below.

        void wlf_register_progress_callback (WLF_PROGRESS_CALLBACK progressCB);
        NV_INT32 wlf_create_file (const NV_CHAR *path, WLF_HEADER wlf_header);
        NV_INT32 wlf_open_file (const NV_CHAR *path, WLF_HEADER *wlf_header, NV_INT32 mode);
        NV_INT32 wlf_read_record (NV_INT32 hnd, NV_INT32 recnum, WLF_RECORD *wlf_record, NV_BOOL wave_flag, NV_INT32 ***wave);
        NV_INT32 wlf_append_record (NV_INT32 hnd, WLF_RECORD wlf_record, NV_INT32 **wave, NV_INT32 shot_num);
        NV_INT32 wlf_update_record (NV_INT32 hnd, NV_INT32 recnum, WLF_RECORD wlf_record);
        void wlf_update_header (NV_INT32 hnd, WLF_HEADER wlf_header);
        NV_INT32 wlf_close_file (NV_INT32 hnd);
        NV_INT32 wlf_close_file_ratio (NV_INT32 hnd, NV_FLOAT32 *ratio, NV_INT32 *total_bytes);
        NV_INT32 wlf_get_errno ();
        NV_CHAR *wlf_strerror ();
        void wlf_perror ();
        void wlf_dump_record (NV_INT32 hnd, WLF_RECORD wlf_record, NV_INT32 **wave);
        NV_CHAR *wlf_get_classification (NV_U_INT16 classification);



    5) SAMPLE CREATION CODE

    In order to create the ASCII header shown above you would need code similar to the following:


        static void waveCatProgress (NV_INT32 state __attribute__ ((unused)), NV_INT32 percent)
          {
            fprintf (stdout, "%03d%% of waveforms appended to file\r", percent);
            fflush (stdout);
            if (percent == 100)
              {
                fprintf (stdout, "                                         \r");
                fflush (stdout);
              }
          }


        WLF_HEADER wlf_header;
        WLF_RECORD wlf_record;
        NV_INT32 **wlf_waveforms;
        NV_INT32 i, j, wlf_handle, percent = 0, old_percent = -1;
        NV_CHAR path[512];


        wlf_register_progress_callback (waveCatProgress);

        .
        .
        .
        //  This will zero out anything we don't specifically set.  //

        memset (&wlf_header, 0, sizeof (WLF_HEADER));

        strcpy (wlf_header.security_classification, "RIDICULOUS");
        strcpy (wlf_header.distribution, "Destroy before reading.");
        strcpy (wlf_header.declassification, "When hell freezes over.");
        strcpy (wlf_header.class_just, "Because I said so.");
        strcpy (wlf_header.downgrade, "Yeah, right.");

        strcpy (wlf_header.source, "JALBTCX");
        strcpy (wlf_header.project, "NM_Saipan");
        strcpy (wlf_header.mission, "02MD06005_004_060320_2225");
        strcpy (wlf_header.dataset, "02DS06005_004_060320_2225_A");
        sprintf (wlf_header.system, "System 2, Type 1, Rate 1000");
        sprintf (wlf_header.flight_id, "Line 7-2");
        strcpy (wlf_header.wlf_user_flag_name[0], "Shallow processed");
        strcpy (wlf_header.wlf_user_flag_name[1], "Shoreline depth swapped");
        strcpy (wlf_header.wlf_user_flag_name[2], "Second depth present");
        strcpy (wlf_header.wlf_user_flag_name[3], "Land");

        strcpy (wlf_header.attr_name[0], "Abbreviated Depth Confidence");
        wlf_header.min_attr[0] = -100.0;
        wlf_header.max_attr[0] = 100.0;
        wlf_header.attr_scale[0] = 1.0;

        strcpy (wlf_header.attr_name[1], "Full Depth Confidence");
        wlf_header.min_attr[1] = .0;
        wlf_header.max_attr[1] = 1000000.0;
        wlf_header.attr_scale[1] = 1.0;

        strcpy (wlf_header.comments, "Blah, blah, blah, yackity smackity");

        wlf_header.flight_start_tv_sec = some value;
        wlf_header.flight_start_tv_nsec = some value;
        wlf_header.flight_end_tv_sec = some value;
        wlf_header.flight_end_tv_nsec = some value;

        wlf_header.z_units = WLF_METERS;
        wlf_header.null_z_value = -998.0;
        wlf_header.max_number_of_returns = 1;

        strcpy (wlf_header.wkt, "COMPD_CS[\"WGS84 with ellipsoid Z\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]],VERT_CS[\"ellipsoid Z in meters\",VERT_DATUM[\"Ellipsoid\",2002],UNIT[\"metre\",1],AXIS[\"Z\",UP]]]");


        wlf_header.min_x = 145.68753660000;
        wlf_header.max_x = 145.70073575000;
        wlf_header.x_scale = 100000000.0;
        wlf_header.min_y = 15.21467045000;
        wlf_header.max_y = 15.22998887000;
        wlf_header.y_scale = 100000000.0;
        wlf_header.min_z = -8388.0;
        wlf_header.max_z = 8388.0;
        wlf_header.z_scale = 1000.0;
        wlf_header.sensor_min_x = 145.68753660000;
        wlf_header.sensor_max_x = 145.70073575000;
        wlf_header.sensor_x_scale = 100000000.0;
        wlf_header.sensor_min_y = 15.21467045000;
        wlf_header.sensor_max_y = 15.22998887000;
        wlf_header.sensor_y_scale = 100000000.0;
        wlf_header.sensor_min_z = -8388.0;
        wlf_header.sensor_max_z = 8388.0;
        wlf_header.sensor_z_scale = 1000.0;
        wlf_header.sensor_min_roll = -30.0;
        wlf_header.sensor_max_roll = 30.0;
        wlf_header.sensor_roll_scale = 10.0;
        wlf_header.sensor_min_pitch = -30.0;
        wlf_header.sensor_max_pitch = 30.0;
        wlf_header.sensor_pitch_scale = 10.0;
        wlf_header.sensor_min_heading = 0.0;
        wlf_header.sensor_max_heading = 360.0;
        wlf_header.sensor_heading_scale = 10.0;
        wlf_header.min_scan_angle = -90.0;
        wlf_header.max_scan_angle = 90.0;
        wlf_header.scan_angle_scale = 10.0;
        wlf_header.min_nadir_angle = 0.0;
        wlf_header.max_nadir_angle = 30.0;
        wlf_header.nadir_angle_scale = 10.0;
        wlf_header.min_water_surface = -8388.0;
        wlf_header.max_water_surface = 8388.0;
        wlf_header.water_surface_scale = 1000.0;
        wlf_header.min_z_offset = -250.0;
        wlf_header.max_z_offset = 250.0;
        wlf_header.z_offset_scale = 1000.0;
        strcpy (wlf_header.z_offset_datum, "Local MLLW defined by tide gage data");
        wlf_header.min_horizontal_uncertainty = 0.0;
        wlf_header.max_horizontal_uncertainty = 10.0;
        wlf_header.horizontal_uncertainty_scale = 1000.0;
        wlf_header.min_vertical_uncertainty = 0.0;
        wlf_header.max_vertical_uncertainty = 10.0;
        wlf_header.vertical_uncertainty_scale = 1000.0;
        wlf_header.number_of_waveforms = 4;
        strcpy (wlf_header.waveform_name[0], "PMT");
        strcpy (wlf_header.waveform_name[1], "APD");
        strcpy (wlf_header.waveform_name[2], "IR");
        strcpy (wlf_header.waveform_name[3], "Raman");
        for (i = 0 ; i < wlf_header.number_of_waveforms ; i++)
          {
            wlf_header.min_waveform[i] = 0;
            wlf_header.max_waveform[i] = 255;
          }
        wlf_header.waveform_count[0] = 501;
        wlf_header.waveform_count[1] = 201;
        wlf_header.waveform_count[2] = 201;
        wlf_header.waveform_count[3] = 81;

        if ((wlf_handle = wlf_create_file (path, wlf_header)) < 0)
          {
            wlf_perror ();
            exit (-1);
          }
        .
        .
        .

    This is the bare minimum you will need to create the file.  You can fill the optional fields if you want to.
    Granted, the above chunk of code is a pain to set up but you will only have to do that on creation of the
    file.  When you read the file the above values will be used to determine how to read the data.

    Note that we didn't have to set intensity_scale, reflectance_scale, max_rgb, max_point_source_id, and
    edge_flag_present to 0, 0.0, or NVFalse since we set the entire header to 0 (using memset) prior to populating
    the fields.


    If we created the header in the code above, the following is example pseudo code for populating the file
    after creation:

        .
        .
        .
        open input file;


        wlf_waveforms = (NV_INT32 **) calloc (4, sizeof (NV_INT32 *));
        if (wlf_waveforms == NULL)
          {
            perror ("Allocating wlf_waveforms in main.c");
            exit (-1);
          }

        for (i = 0 ; i < wlf_header.number_of_waveforms ; i++)
          {
            wlf_waveforms[i] = (NV_INT32 *) calloc (wlf_header.waveform_count[i], sizeof (NV_INT32));
            if (wlf_waveforms[i] == NULL)
              {
                perror ("Allocating wlf_waveforms[i] in main.c");
                exit (-1);
              }
          }

        for (i = 0 ; i < number of shots ; i++)
          {
            Read shot number i;

            wlf_record.tv_sec = seconds from the epoch;
            wlf_record.tv_nsec = nanoseconds of the second;
            wlf_record.horizontal_uncertainty = some value;
            wlf_record.vertical_uncertainty = some value;
            wlf_record.x = some value;
            wlf_record.y = some value;
            wlf_record.z = some value;
            wlf_record.waveform = waveform number from which this point was computed
            wlf_record.waveform_point = index within the waveform at which this point was computed
            wlf_record.sensor_x = some value;
            wlf_record.sensor_y = some value;
            wlf_record.sensor_z = some value;
            wlf_record.sensor_roll = some value;
            wlf_record.sensor_pitch = some value;
            wlf_record.sensor_heading = some value;
            wlf_record.scan_angle = some value;
            wlf_record.nadir_angle = some value;
            wlf_record.water_surface = some value;
            wlf_record.z_offset = some value that will be added to wlf_record.z and wlf_record.water_surface;
            wlf_record.number_of_returns = 1;
            wlf_record.return_number = 1;
            wlf_record.attribute[0] = some value;
            wlf_record.attribute[1] = some value;
            wlf_record.classification = some value;
            wlf_record.status = some combination of status flags;

            for (i = 0 ; i < wlf_header.number_of_waveforms ; i++)
              {
                for (j = 0 ; j < wlf_header.waveform_count[i] ; j++)
                  {
                    wlf_waveform[i][j] = some waveform value [j];
                  }
              }


            //  All fields will be range checked in the API, a negative return indicates an error.  //

            if (wlf_append_record (wlf_handle, wlf_record, wlf_waveforms, i) < 0)
              {
                wlf_perror ();
                exit (-1);
              }


            //  Percent load spinner for user warm/fuzzy.  //

            percent = (NV_INT32) (((NV_FLOAT32) i / (NV_FLOAT32) number of shots) * 100.0);
            if (percent != old_percent)
              {
                fprintf (stdout, "%03d%% of data loaded\r", percent);
                fflush (stdout);

                old_percent = percent;
              }
          }

          fprintf (stdout, "                                          \r");
          fflush (stdout);


          for (i = 0 ; i < wlf_header.number_of_waveforms ; i++) free (wlf_waveforms[i]);
          free (wlf_waveforms);

          close input file;

          if (wlf_close_file_ratio (wlf_handle, &ratio, &total_bytes) < 0)
            {
              wlf_perror ();
              exit (-1);
            }
          printf ("Waveform compression ratio = %f\n\n", ratio);
          .
          .
          .




    6) SAMPLE READING CODE

        The following is a pseudo-code example of how you might read a WLF file:


        WLF_HEADER     wlf_header;
        WLF_RECORD     wlf_record;
        NV_INT32       **waveforms = NULL;
        NV_INT32       i, j, k, wlf_handle;
        NV_CHAR        path[512];
        .
        .
        .
        if ((wlf_handle = wlf_open_file (path, &wlf_header, WLF_READONLY)) < 0)
          {
            wlf_perror ();
            exit (-1);
          }


        //  Check for newer file version.  //

        if (wlf_get_errno () == WLF_NEWER_FILE_VERSION_WARNING)
          fprintf (stderr, "\n\nWarning, the file version is newer than the library version.\nPlease upgrade your library.\n\n");


        for (i = 0 ; i < wlf_header.number_of_records ; i++)
          {
            //  If you don't have or want waveforms, set the 4th argument to NVFalse.  //

            if (wlf_read_record (wlf_handle, i, &wlf_record, NVTrue, &waveforms) < 0)
              {
                wlf_perror ();
                exit (-1);
              }


            //
            Do something with the data, for instance - wlf_dump_record (wlf_handle, wlf_record, waveforms).
            You can use the optional field presence flags to determine what optional record fields have been populated
            in the file, for example:
            //

            if (wlf_header.opt.reflectance_present) Do something with the reflectance;
            if (wlf_header.opt.rgb_present) Do something with the RGB values;
            if (wlf_header.opt.z_offset_present)
              {
                original_z_value = wlf_record.z - wlf_record.z_offset;
                if (wlf_header.opt.water_surface_present) 
                  original_water_surface_value = wlf_record.water_surface - wlf_record.z_offset;
              }
            if (wlf_header.sensor_position_present) Do something with the sensor_x, sensor_y, sensor_z values;
            if (wlf_header.sensor_attitude_present) Do something with the sensor_roll, sensor_pitch, sensor_heading values;


            //  Optional attributes  //

            for (j = 0 ; j < WLF_MAX_ATTR ; j++)
              {
                if (wlf_heade.opt.attr_present[j]) Do something with the attribute;
              }


            //  If waveforms are present, do something with the waveforms.  //

            if (wlf_header.number_of_waveforms)
              {
                for (j = 0 ; j < wlf_header.number_of_waveforms ; j++)
                  {
                    for (k = 0 ; k < wlf_header.waveform_count[j] ; k++)
                      {
                        //  Do something with the waveforms - waveforms[j][k]  //

                        if (j == wlf_record.waveform && k == wlf_record.waveform_point) // mark the selected point on the waveform //
                      }
                  }
              }
          }

        if (wlf_close_file (wlf_handle) < 0)
          {
            wlf_perror ();
            exit (-1);
          }
        .
        .
        .




    7)  Explanation of Z offsets

        In most cases the data that you wish to store will be defined against a well-known vertical datum
        such as NAVD88 or the WGS84 ellipsoid.  In these cases there is usually no need to define any Z
        offsets in the data set.  Unfortunately, the Navy (and sometimes the Army Corps) is required to
        survey in an area where the local datum has not been defined.  Obviously, in that case, a well-known
        text description of the local datum model or the differences between the local datum and a well-known
        vertical datum is not available.  This is normally only a problem when doing bathymetric/hydrographic
        surveys.

        So, what do you do then?  The most common solution is to install tide gages, survey them in so that
        their relationship to a well-known vertical datum (usually WGS84) can be established, and then collect
        tide data for, at the very least, 15 days to establish MLLW or MSL at that location.  After that you
        can take the offsets between MLLW or MSL at each of the tide gages and develop a model (usually just 
        a grid of differences) of the difference between MLLW (or whatever) and the well-known vertical datum.
        You then apply the model to the collected data to shift it to the local datum.

        This is where the problem comes in.  Since your model of the ellipsoid-MLLW difference is not a
        well-known text defined model (and not likely to become one) you have to save these offsets in the
        file so that you can, at some later time (for instance, when a really good model of ellipsoid-MLLW
        difference becomes available), revert to the well-known vertical datum.

        The {WELL-KNOWN TEXT field of the header should describe the data without the offset applied. 
        Normally this would be the data as collected in relation to the well-known vertical datum (like WGS84).
        So, how do you describe the local datum in the WLF file?  You could just put in another
        {WELL-KNOWN TEXT field but that seems like overkill.  After all, all you're really trying to do is
        describe the local vertical datum.  To do that you need to provide a description to be stored in the
        header.  That description is stored in the [Z OFFSET DATUM] field.

        Since the way you will most often want to access the data is in relation to the local datum the local
        datum adjusted values will actually be stored in the file.  When you create the file you need to
        deliver the initial value (as collected) and the offset value that you want to ADD to the initial
        value to get the adjusted value.  These values should be placed in wlf_record.z,
        wlf_record.water_surface (if available), and wlf_record.z_offset.

        When you retrieve the data you will get the adjusted value returned in wlf_record.z and the offset
        value in wlf_record.z_offset.  To revert to the well-known vertical datum just SUBTRACT 
        wlf_record.z_offset from wlf_record.z (do the same for water_surface if needed).  Pay very close
        attention to the fact that you must supply the value that is to be ADDED to the original value.

        On the bright side, if you have a well-known vertical datum you don't even need to worry about this
        (unless you might want to provide two values) and can forget about storing offsets.




    8) CAVEATS

        *   The API handles ALL scaling of data.  That is, the application should not scale anything prior
            to appending each record.  The idea being, if you handed the API a Z value of -50.0727 and had
            [MIN Z] at -8388.0, [MAX Z] at 8388.0, and [Z SCALE] at 1000.0, the API will scale it, offset it,
            and store it.  On retrieval (read) of the data, the API will hand you the value 50.073 or possibly
            50.0729999999 depending on round off.

        *   ALL record values, including waveform values, will be range checked against the provided range
            when appended to the file on creation.  This slows things down a bit but makes sure that you have
            not tried to insert a bad value.

        *   This format is designed for single flight files.  You can stuff more than one flight into a file
            but then you will invalidate some of the header fields (like [FLIGHT START DAY]).  If you need to
            concatenate a bunch of files remember that appending to previously created files is not allowed.
            You can only append to newly created files.  Once a newly created file is closed, appending is no
            longer possible.

        *   Only the status, classification, reflectance, rgb, and attribute fields of each record will be
            modifiable.  The reason for this is that this is a transfer/archive format.  The data itself
            should not be modified.  The status (which contains the validity and other flags), the
            classification, the reflectance values, the RGB values, and, obviously, the attribute values
            may be interpreted differently by different organizations but the bulk of the data should remain
            as collected.

        *   The status field pertains to the entire record and the associated waveforms.  That is, if you set
            WLF_MANUALLY_INVAL then the entire record is invalid.  If you want to do something else you may want
            to try using the WLF_USER_NN status flags.

        *   Only a few of the header fields are modifiable (see the structure footnotes below).

        *   Some of the header fields are populated by the API (see the structure footnotes below).

        *   Z values are always stored as elevations.

        *   Time will always be UTC.  Local time makes no sense whatsoever.

        *   You are limited to a maximum of 16,777,216 total bytes for each waveform block (I hope that's enough ;-)

        *   Waveform values are limited to a signed, 32 bit integer value (-2,147,483,648 to 2,147,483,647)

        *   The current limit on different types of waveforms is 20.

        *   The current limit on open WLF files is 128.

        *   RECORD NUMBERS START AT ZERO (0) NOT ONE (1) !!!!!

        *   NEVER, EVER, change WLF_MAX_ATTR since that would ruin our "forward compatibility".  If you really
            need to define more than 10 optional attributes you're definitley using the wrong format.  These were
            added to the record to allow for unforseen circumstances.  As a transfer/archive format we probably
            won't be using these very much.

        *   While there is no file extension check in the WLF API the accepted file extensions for WLF are .wlf 
            for generic WLF files, .wtf for topo only WLF files, and .whf for hydro only WLF files.


*/



#ifndef __WLF_H__
#define __WLF_H__


#ifdef  __cplusplus
extern "C" {
#endif


  /*  Preparing for language translation using GNU gettext at some point in the future.  */

#define _(String) (String)
#define N_(String) String

  /*
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
  */



  /*  Windoze insanity.  Note that there may be nothing defined for any of these DLL values.  When building statically on Windows I
      strip out the __declspec (dllexport) and __declspec (dllimport) definitions before I copy the .h file to the central include
      location.  That's so I don't have to modify a bunch of application Makefiles to check for static build adn set the _STATIC
      option.  Of course, on Linux/UNIX we don't have to do anything to these ;-)  */

#ifdef WLF_DLL_EXPORT
#  define WLF_DLL __declspec(dllexport)
#else
#  ifdef NVWIN3X
#    ifdef WLF_STATIC
#      define WLF_DLL
#    else
#      define WLF_DLL __declspec(dllimport)
#    endif
#  else
#    define WLF_DLL
#  endif
#endif



#include "wlf_nvtypes.h"
#include "wlf_class.h"


#define WLF_MAX_FILES               128  /*  Maximum number of open WLF files.  */
#define WLF_HEADER_SIZE             65536
#define WLF_MAX_WAVEFORMS           20   /*  Maximum number of waveforms.  */
#define WLF_MAX_ATTR                10   /*  NEVER CHANGE THIS NUMBER !!!  If you need more than 10 find another format.  */


  /*  Version 1.0 default variable bit sizes.  */

#define WLF_TV_SEC_BITS             32   /*  Whenever POSIX time goes to 64 bits we can increase this to 33 or 34.  */
#define WLF_TV_NSEC_BITS            31
#define WLF_CLASS_BITS              12
#define WLF_STATUS_BITS             11
#define WLF_ADDRESS_BITS            36
#define WLF_WAVEFORM_BLOCKSIZE_BITS 24   /*  If this value needs to be increased it must go to 32.  */


  /*  Version 1.0 units.  */

#define WLF_METERS                  0
#define WLF_FEET                    1
#define WLF_FATHOMS                 2


  /*  File open modes.  */

#define WLF_UPDATE                  0
#define WLF_READONLY                1


  /*  

    Status bit definitions:

    WLF_MANUALLY_INVAL    -  Point has been manually marked as invalid.
    WLF_FILTER_INVAL      -  Point has been automatically marked as invalid.
    WLF_SELECTED_SOUNDING -  Point is a selected sounding (min, max, avg, depends on application).
    WLF_SELECTED_FEATURE  -  Point is on or near a selected feature.
    WLF_DESIGNATED_SOUNDING -  Indicates the depth record overrides all hypotheses.  Not all 'designated' soundings
                             will be 'features'.  This is included to support the Open Navigation Surface (ONS)
                             Bathymetric Attributed Grid (BAG or Big Ass Grid) standard format.
    WLF_SUSPECT           -  Point has been marked as suspect.
    WLF_USER_01           -  User flag 01.
    WLF_USER_02           -  User flag 02.
    WLF_USER_03           -  User flag 03.
    WLF_USER_04           -  User flag 04.
    WLF_USER_05           -  User flag 05.

  */


#define       WLF_MANUALLY_INVAL       1       /* 0000 0000 0001 */
#define       WLF_FILTER_INVAL         2       /* 0000 0000 0010 */
#define       WLF_SELECTED_SOUNDING    4       /* 0000 0000 0100 */
#define       WLF_SELECTED_FEATURE     8       /* 0000 0000 1000 */
#define       WLF_DESIGNATED_SOUNDING  16      /* 0000 0001 0000 */
#define       WLF_SUSPECT              32      /* 0000 0010 0000 */
#define       WLF_USER_01              64      /* 0000 0100 0000 */
#define       WLF_USER_02              128     /* 0000 1000 0000 */
#define       WLF_USER_03              256     /* 0001 0000 0000 */
#define       WLF_USER_04              512     /* 0010 0000 0000 */
#define       WLF_USER_05              1024    /* 0100 0000 0000 */

#define       WLF_INVAL                3       /* 0000 0000 0011 */
#define       WLF_SELECTED             28      /* 0000 0001 1100 */
#define       WLF_USER                 1984    /* 0111 1100 0000 */


  /*  Error conditions.  */

#define       WLF_SUCCESS                      0
#define       WLF_WAVE_WRITE_FSEEK_ERROR       -1
#define       WLF_WAVE_WRITE_ERROR             -2
#define       WLF_RECORD_WRITE_FSEEK_ERROR     -3
#define       WLF_RECORD_WRITE_ERROR           -4
#define       WLF_HEADER_WRITE_FSEEK_ERROR     -5
#define       WLF_HEADER_WRITE_ERROR           -6
#define       WLF_TOO_MANY_OPEN_FILES          -7
#define       WLF_CREATE_ERROR                 -8
#define       WLF_CREATE_WAVE_ERROR            -9
#define       WLF_OPEN_UPDATE_ERROR            -10
#define       WLF_OPEN_READONLY_ERROR          -11
#define       WLF_NOT_WLF_FILE_ERROR           -12
#define       WLF_NEWER_FILE_VERSION_WARNING   -13
#define       WLF_CLOSE_FSEEK_ERROR            -14
#define       WLF_CLOSE_WAVE_FSEEK_ERROR       -15
#define       WLF_CLOSE_WAVE_READ_ERROR        -16
#define       WLF_CLOSE_WAVE_WRITE_ERROR       -17
#define       WLF_CLOSE_ERROR                  -18
#define       WLF_INVALID_RECORD_NUMBER        -19
#define       WLF_READ_FSEEK_ERROR             -20
#define       WLF_READ_ERROR                   -21
#define       WLF_APPEND_NOT_CREATING_ERROR    -22
#define       WLF_APPEND_FSEEK_ERROR           -23
#define       WLF_WAVE_READ_FSEEK_ERROR        -24
#define       WLF_WAVE_READ_ERROR              -25
#define       WLF_UPDATE_RECORD_FSEEK_ERROR    -26
#define       WLF_UPDATE_RECORD_READ_ERROR     -27
#define       WLF_UPDATE_RECORD_WRITE_ERROR    -28
#define       WLF_VALUE_OUT_OF_RANGE_ERROR     -29
#define       WLF_SENSOR_POSITION_RANGE_ERROR  -30
#define       WLF_SENSOR_ATTITUDE_RANGE_ERROR  -31



  /*  Optional WLF_RECORD field presence flags.  */

  typedef struct
  {
    NV_BOOL         horizontal_uncertainty_present;
    NV_BOOL         vertical_uncertainty_present;
    NV_BOOL         sensor_position_present;
    NV_BOOL         sensor_attitude_present;
    NV_BOOL         scan_angle_present;
    NV_BOOL         nadir_angle_present;
    NV_BOOL         water_surface_present;
    NV_BOOL         z_offset_present;
    NV_BOOL         point_source_present;
    NV_BOOL         edge_of_flight_line_present;
    NV_BOOL         intensity_present;
    NV_BOOL         attr_present[WLF_MAX_ATTR];
    NV_BOOL         rgb_present;
    NV_BOOL         reflectance_present;
  } OPTIONAL_FIELDS;



  /*  Header structure.  */

  typedef struct
  {
    NV_CHAR         version[128];                 /*  Library version information  (0)  */
    NV_U_INT32      file_source_id;               /*  Provided to allow easier conversion to LAS (1)  */
    time_t          creation_tv_sec;              /*  File creation POSIX seconds from 01-01-1970  (0)  */
    long            creation_tv_nsec;             /*  File creation POSIX nanoseconds of second from 01-01-1970  (0)  */
    NV_CHAR         creation_software[128];       /*  File creation software  (1)  */
    time_t          modification_tv_sec;          /*  File modification POSIX seconds from 01-01-1970  (0)  */
    long            modification_tv_nsec;         /*  File modification POSIX nanoseconds of second from 01-01-1970  (0)  */
    NV_CHAR         modification_software[128];   /*  File modification software  (1)(9)  */
    NV_CHAR         security_classification[32];  /*  Security classification  (1)(9)  */
    NV_CHAR         distribution[1024];           /*  Security distribution statement  (1)(9)  */
    NV_CHAR         declassification[256];        /*  Security declassification statement  (1)(9)  */
    NV_CHAR         class_just[256];              /*  Security classification justification  (1)(9)  */
    NV_CHAR         downgrade[128];               /*  Security downgrade statement  (1)(9)  */
    NV_CHAR         source[128];                  /*  Source of data  (1)  */
    NV_CHAR         system[128];                  /*  Collection system  (1)  */
    NV_FLOAT64      obs_min_x;                    /*  Minimum X value in file  (0)  */
    NV_FLOAT64      obs_min_y;                    /*  Minimum Y value in file  (0)  */
    NV_FLOAT32      obs_min_z;                    /*  Minimum Z value in file  (0)  */
    NV_FLOAT64      obs_max_x;                    /*  Maximum X value in file  (0)  */
    NV_FLOAT64      obs_max_y;                    /*  Maximum Y value in file  (0)  */
    NV_FLOAT32      obs_max_z;                    /*  Maximum Z value in file  (0)  */
    NV_FLOAT64      obs_valid_min_x;              /*  Minimum valid X value in file  (0)  */
    NV_FLOAT64      obs_valid_min_y;              /*  Minimum valid Y value in file  (0)  */
    NV_FLOAT32      obs_valid_min_z;              /*  Minimum valid Z value in file  (0)  */
    NV_FLOAT64      obs_valid_max_x;              /*  Maximum valid X value in file  (0)  */
    NV_FLOAT64      obs_valid_max_y;              /*  Maximum valid Y value in file  (0)  */
    NV_FLOAT32      obs_valid_max_z;              /*  Maximum valid Z value in file  (0)  */
    NV_U_INT32      number_of_records;            /*  Number of records in file  (0)  */
    NV_CHAR         project[128];                 /*  Project information  (1)  */
    NV_CHAR         mission[128];                 /*  Mission information  (1)  */
    NV_CHAR         dataset[128];                 /*  Dataset information  (1)  */
    NV_CHAR         flight_id[128];               /*  Flight information  (1)  */
    time_t          flight_start_tv_sec;          /*  Start of flight POSIX seconds from 01-01-1970  (1)  */
    long            flight_start_tv_nsec;         /*  Start of flight POSIX nanoseconds of second from 01-01-1970  (1)  */
    time_t          flight_end_tv_sec;            /*  End of flight POSIX seconds from 01-01-1970  (1)  */
    long            flight_end_tv_nsec;           /*  End of flight POSIX nanoseconds of second from 01-01-1970  (1)  */
    NV_CHAR         wlf_user_flag_name[5][32];    /*  WLF user/application defined status flag names  (1)  */
    NV_CHAR         comments[4096];               /*  Comments  (1)(9)  */
    OPTIONAL_FIELDS opt;                          /*  Convenience flags to show whether an optional field is present.  (0)(13)  */


    /*  The following fields will be required to create a file and will not be modifiable after creation of the file.
        If you do not want to include a data type (e.g. intensity, uncertainty, water_surface) set the ???_scale
        value to 0.0.  If you do not want waveforms, set number_of_waveforms to 0.  See footnotes for more info.*/

    NV_U_BYTE       z_units;                      /*  Z units, either WLF_METERS, WLF_FEET, or WLF_FATHOMS  (2)  */
    NV_FLOAT32      null_z_value;                 /*  Null value to be used for Z and water_surface  (2)  */
    NV_U_BYTE       max_number_of_returns;        /*  Maximum number of returns per shot  (2)  */
    NV_CHAR         wkt[1024];                    /*  OpenGIS Well-known text datum information  (2)  */
    NV_FLOAT64      min_x;                        /*  Minimum possible X value for file  (2)  */
    NV_FLOAT64      max_x;                        /*  Maximum possible X value for file  (2)  */
    NV_FLOAT64      x_scale;                      /*  Scale to multiply X values by  (2)(4)  */
    NV_FLOAT64      min_y;                        /*  Minimum possible Y value for file  (2)  */
    NV_FLOAT64      max_y;                        /*  Maximum possible Y value for file  (2)  */
    NV_FLOAT64      y_scale;                      /*  Scale to multiply Y values by  (2)(4)  */
    NV_FLOAT32      min_z;                        /*  Minimum possible Z value for file  (2)  */
    NV_FLOAT32      max_z;                        /*  Maximum possible Z value for file  (2)  */
    NV_FLOAT32      z_scale;                      /*  Scale to multiply Z values by  (2)(4)  */
    NV_FLOAT64      sensor_min_x;                 /*  Minimum possible sensor X value for file  (3)(14)  */
    NV_FLOAT64      sensor_max_x;                 /*  Maximum possible sensor X value for file  (3)(14)  */
    NV_FLOAT64      sensor_x_scale;               /*  Scale to multiply sensor X values by  (3)(4)(14)  */
    NV_FLOAT64      sensor_min_y;                 /*  Minimum possible sensor Y value for file  (3)(14)  */
    NV_FLOAT64      sensor_max_y;                 /*  Maximum possible sensor Y value for file  (3)(14)  */
    NV_FLOAT64      sensor_y_scale;               /*  Scale to multiply sensor Y values by  (3)(4)(14)  */
    NV_FLOAT32      sensor_min_z;                 /*  Minimum possible sensor Z value for file  (3)(14)  */
    NV_FLOAT32      sensor_max_z;                 /*  Maximum possible sensor Z value for file  (3)(14)  */
    NV_FLOAT32      sensor_z_scale;               /*  Scale to multiply sensor Z values by  (3)(4)(14)  */
    NV_FLOAT32      sensor_min_roll;              /*  Minimum possible sensor roll value for file  (3)(15)  */
    NV_FLOAT32      sensor_max_roll;              /*  Maximum possible sensor roll value for file  (3)(15)  */
    NV_FLOAT32      sensor_roll_scale;            /*  Scale to multiply sensor roll values by  (3)(4)(15)  */
    NV_FLOAT32      sensor_min_pitch;             /*  Minimum possible sensor pitch value for file  (3)(15)  */
    NV_FLOAT32      sensor_max_pitch;             /*  Maximum possible sensor pitch value for file  (3)(15)  */
    NV_FLOAT32      sensor_pitch_scale;           /*  Scale to multiply sensor pitch values by  (3)(4)(15)  */
    NV_FLOAT32      sensor_min_heading;           /*  Minimum possible sensor heading value for file  (3)(15)  */
    NV_FLOAT32      sensor_max_heading;           /*  Maximum possible sensor heading value for file  (3)(15)  */
    NV_FLOAT32      sensor_heading_scale;         /*  Scale to multiply sensor heading values by  (3)(4)(15)  */
    NV_FLOAT32      min_scan_angle;               /*  Minimum possible scan angle value for file  (3)(5)  */
    NV_FLOAT32      max_scan_angle;               /*  Maximum possible scan angle value for file  (3)(5)  */
    NV_FLOAT32      scan_angle_scale;             /*  Scale to multiply scan angle values by  (3)(4)  */
    NV_FLOAT32      min_nadir_angle;              /*  Minimum possible nadir angle value for file  (3)  */
    NV_FLOAT32      max_nadir_angle;              /*  Maximum possible nadir angle value for file  (3)  */
    NV_FLOAT32      nadir_angle_scale;            /*  Scale to multiply nadir angle values by  (3)(4)  */
    NV_FLOAT32      min_water_surface;            /*  Minimum possible water surface value for file  (3)  */
    NV_FLOAT32      max_water_surface;            /*  Maximum possible water surface value for file  (3)  */
    NV_FLOAT32      water_surface_scale;          /*  Scale to multiply water surface values by  (3)(4)  */
    NV_FLOAT32      min_z_offset;                 /*  Minimum possible Z offset value for file  (3)(10)  */
    NV_FLOAT32      max_z_offset;                 /*  Maximum possible Z offset value for file  (3)(10)  */
    NV_FLOAT32      z_offset_scale;               /*  Scale to multiply Z offset values by  (3)(4)(10)  */
    NV_CHAR         z_offset_datum[128];          /*  Z offset vertical datum.  (12)  */
    NV_FLOAT32      min_intensity;                /*  Minimum possible intensity value for file  (3)  */
    NV_FLOAT32      max_intensity;                /*  Maximum possible intensity value for file  (3)  */
    NV_FLOAT32      intensity_scale;              /*  Scale to multiply intensity values by  (3)(4)  */
    NV_CHAR         attr_name[WLF_MAX_ATTR][64];  /*  Optional attribute names (3)  */
    NV_FLOAT32      min_attr[WLF_MAX_ATTR];       /*  Minimum possible value for optional attribute for file  (3)  */
    NV_FLOAT32      max_attr[WLF_MAX_ATTR];       /*  Maximum possible value for optional attribute for file  (3)  */
    NV_FLOAT32      attr_scale[WLF_MAX_ATTR];     /*  Scale to multiply optional attribute values by  (3)(4)  */
    NV_FLOAT32      min_reflectance;              /*  Minimum possible reflectance value for file  (3)  */
    NV_FLOAT32      max_reflectance;              /*  Maximum possible reflectance value for file  (3)  */
    NV_FLOAT32      reflectance_scale;            /*  Scale to multiply reflectance values by  (3)(4)  */
    NV_FLOAT32      min_horizontal_uncertainty;   /*  Minimum possible horizontal uncertainty value for file  (3)  */
    NV_FLOAT32      max_horizontal_uncertainty;   /*  Maximum possible horizontal uncertainty value for file  (3)  */
    NV_FLOAT32      horizontal_uncertainty_scale; /*  Scale to multiply horizontal uncertainty values by  (3)(4)  */
    NV_FLOAT32      min_vertical_uncertainty;     /*  Minimum possible vertical uncertainty value for file  (3)  */
    NV_FLOAT32      max_vertical_uncertainty;     /*  Maximum possible vertical uncertainty value for file  (3)  */
    NV_FLOAT32      vertical_uncertainty_scale;   /*  Scale to multiply vertical uncertainty values by  (3)(4)  */
    NV_U_INT32      max_rgb;                      /*  Maximum possible RGB value for file  (2)(11)  */
    NV_U_INT32      max_point_source_id;          /*  Maximum possible point source ID number for file  (2)(11)  */
    NV_BOOL         edge_flag_present;            /*  Set to NVTrue if edge flags have been included in data (else NVFalse)  (2)  */
    NV_U_BYTE       number_of_waveforms;          /*  Number of associated waveforms to be stored for each record  (6)  */
    NV_CHAR         waveform_name[WLF_MAX_WAVEFORMS][32]; /*  Array of waveform names  (7)  */
    NV_INT32        min_waveform[WLF_MAX_WAVEFORMS]; /*  Array of minimum possible waveform values for file  (8)  */
    NV_INT32        max_waveform[WLF_MAX_WAVEFORMS]; /*  Array of maximum possible waveform values for file  (8)  */
    NV_INT32        waveform_count[WLF_MAX_WAVEFORMS]; /*  Array containing number of waveform points per waveform  (8)  */
  } WLF_HEADER;

  /*
      WLF_HEADER footnotes:

      (0)  - Populated by the API.
      (1)  - Optional.
      (2)  - Required on creation of file.  Not modifiable after creation.
      (3)  - Required on creation of file unless scale value is set to 0.0.  Not modifiable after creation.
      (4)  - Scale values are essentially the required resolution.  Normally they would be something like 1000.0
             but they could be any positive value, like 3.14159.  This value, in conjuction with the min and 
             max values is used to determine storage requirements (in bits).
      (5)  - Scan angles are left negative
      (6)  - Set this value to 0 if there are no associated waveforms or you do not wish to store waveforms.
      (7)  - Max of 32 characters, required if number_of_waveforms is non-zero.
      (8)  - Integer values, the minimum is normally zero but may be negative.  Waveform arrays, which may be read along
             with the WLF_RECORD, are defined as **NV_INT32.
      (9)  - Modifiable after creation of the file.  Most descriptive strings in the header are modifiable.
      (10) - See Caveats section on Z offsets above.
      (11) - Set 0 to exclude data from the file.
      (12) - Required if z_offset_scale is not 0.0.
      (13) - These fields are redundant since the presence or absence of a field in the record can be determined
             from other header fields (e.g. z_offset_scale set to 0.0).  They are provided by the API for 
             convenience and speed.
      (14) - If you populate any of the sensor position fields (sensor_x, sensor_y, sensor_z) you must populate all
             three.
      (15) - If you populate any of the sensor attitude fields (sensor_roll, sensor_pitch, sensor_heading) you must populate all
             three.
  */



  /*  Point record structure.  */

  typedef struct
  {
    time_t           tv_sec;                 /*  POSIX seconds of shot from 01-01-1970  (0)  */
    long             tv_nsec;                /*  POSIX nanoseconds of second of shot from 01-01-1970  (0)  */
    NV_FLOAT32       horizontal_uncertainty; /*  In [Z UNITS]  (1)  */
    NV_FLOAT32       vertical_uncertainty;   /*  In [Z UNITS]  (1)  */
    NV_FLOAT64       x;                      /*  X position based on WKT data.  (0)  */
    NV_FLOAT64       y;                      /*  Y position based on WKT data.  (0)  */
    NV_FLOAT32       z;                      /*  Z position based on WKT data.  (0)  */
    NV_U_BYTE        waveform;               /*  Number of waveform from which this point was computed.  (6)  */
    NV_INT32         waveform_point;         /*  Position within the waveform array at which this point was computed.  (6)  */
    NV_FLOAT64       sensor_x;               /*  X position of sensor based on WKT data.  (1)  */
    NV_FLOAT64       sensor_y;               /*  Y position of sensor based on WKT data.  (1)  */
    NV_FLOAT32       sensor_z;               /*  Z position of sensor based on WKT data.  (1)  */
    NV_FLOAT32       sensor_roll;            /*  Sensor roll value in degrees (right-down is positive).  (1)  */
    NV_FLOAT32       sensor_pitch;           /*  Sensor pitch value in degrees (forward-up is positive).  (1)  */
    NV_FLOAT32       sensor_heading;         /*  Sensor heading value in degrees (left-forward is positive [clockwise looking down]).  (1)  */
    NV_FLOAT32       scan_angle;             /*  Left negative.  (1)(5)  */
    NV_FLOAT32       nadir_angle;            /*  Forward positive.  (1)(5)  */
    NV_FLOAT32       water_surface;          /*  Water surface value [Z UNITS]  (1)  */
    NV_FLOAT32       z_offset;               /*  Z and water_surface offset value [Z UNITS]  (1)(3)  */
    NV_U_BYTE        number_of_returns;      /*  Number of returns for this shot.  (0)  */
    NV_U_BYTE        return_number;          /*  Return number of this shot  (0)  */
    NV_U_INT32       point_source;           /*  Numeric source ID from 0 to header.max_point_source_id  (1)(4)  */
    NV_BYTE          edge_of_flight_line;    /*  0 = not edge, -1 = left edge, 1 = right edge.  (1)  */
    NV_FLOAT32       intensity;              /*  Intensity.  (1)  */
    NV_FLOAT32       attribute[WLF_MAX_ATTR];/*  Optional attributes for special cases. (1)(2)  */
    NV_U_INT32       red;                    /*  Red value.  (1)(2)  */
    NV_U_INT32       green;                  /*  Green value.  (1)(2)  */
    NV_U_INT32       blue;                   /*  Blue value.  (1)(2)  */
    NV_FLOAT32       reflectance;            /*  Bottom reflectance or pseudo-reflectance.  (1)(2)  */
    NV_U_INT16       classification;         /*  Bottom or ground classification (undefined at present).  (0)(2)  */
    NV_U_INT16       status;                 /*  11 bits of status information.  (0)(2)  */
  } WLF_RECORD;


  /*
      WLF_RECORD footnotes:

      (0)  - Required.
      (1)  - Optional.
      (2)  - Modifiable after creation of the file.  These fields are open to end user interpretation so we
             need to be able to modify them after file creation.
      (3)  - See Caveats section on Z offsets above.
      (4)  - Provided mostly for ease of conversion to/from LAS.  Can be used for concatenating multiple flights. 
      (5)  - Scan and nadir angles are post attitude application angles.  That is, you should have already applied
             the sensor roll, pitch, and heading adjustments to them prior to loading them into the file.
      (6)  - This value is only populated if waveforms are stored for this file, that is, wlf_header.number_of_waveforms
             is non-zero.
  */



  /*  Progress callback.  */

  typedef void (*WLF_PROGRESS_CALLBACK) (NV_INT32 state, NV_INT32 percent);




  /*  Public API I/O function declarations.  */



  /*********************************************************************************************

    Module Name:        wlf_register_progress_callback

    Programmer(s):      Jan C. Depner

    Date Written:       04/14/09

    Purpose:            Allows the calling application to register a callback procedure that
                        will receive a state and percent complete value from the wlf_close_file
                        waveform concatenation process.  This allows a caller to keep the user 
                        informed about the progress of the last part of creating a new
                        WLF file.  For an example of use see section 4 (SAMPLE CREATION CODE)
                        in the wlf.h file.

    Arguments:          progressCB - callback function

    Return Value:       None

    Caveats:            At present the state variable is not used and is set to 0.  If we add 
                        other callbacks in the future the state will be set to 1, 2, 3, etc
                        for each type needed.

  *********************************************************************************************/

  WLF_DLL void wlf_register_progress_callback (WLF_PROGRESS_CALLBACK progressCB);





  /*********************************************************************************************

  Function:    wlf_create_file

  Purpose:     Create a WLF file.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   path           -    The WLF file path
               wlf_header     -    WLF_HEADER structure to
                                   be written to the file

  Returns:     NV_INT32       -    The file handle (0 or positive) or
                                   WLF_TOO_MANY_OPEN_FILES
                                   WLF_CREATE_ERROR
                                   WLF_CREATE_WAVE_ERROR
                                   WLF_HEADER_WRITE_FSEEK_ERROR
                                   WLF_HEADER_WRITE_ERROR
                                   WLF_SENSOR_POSITION_RANGE_ERROR
                                   WLF_SENSOR_ATTITUDE_RANGE_ERROR

  *********************************************************************************************/

  WLF_DLL NV_INT32 wlf_create_file (const NV_CHAR *path, WLF_HEADER wlf_header);





  /*********************************************************************************************

  Function:    wlf_open_file

  Purpose:     Open a WLF file.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        09/25/09

  Arguments:   path           -    The WLF file path
               wlf_header     -    WLF_HEADER structure to be populated
               mode           -    WLF_UPDATE or WLF_READ_ONLY

  Returns:     NV_INT32       -    The file handle (0 or positive) or
                                   WLF_TOO_MANY_OPEN_FILES
                                   WLF_OPEN_UPDATE_ERROR
                                   WLF_OPEN_READONLY_ERROR
                                   WLF_NOT_WLF_FILE_ERROR

  Caveats:     wlf_error.wlf may be set to WLF_NEWER_FILE_VERSION_WARNING if the file version
               is newer than the library version.  This shouldn't cause a problem but
               you may want to check it with wlf_get_errno () when you open a file so
               you can issue a warning.

  *********************************************************************************************/

  WLF_DLL NV_INT32 wlf_open_file (const NV_CHAR *path, WLF_HEADER *wlf_header, NV_INT32 mode);





  /*********************************************************************************************

  Function:    wlf_read_record

  Purpose:     Retrieve a WLF point and (optionally) waveforms from a WLF file.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        09/25/09

  Arguments:   hnd            -    The file handle
               recnum         -    The record number of the WLF record to be retrieved
               wlf_record     -    The returned WLF record
               wave_flag      -    Set to NVTrue if you want waveforms
               wave           -    Pointer to the waveform structure

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_INVALID_RECORD_NUMBER
                                   WLF_READ_FSEEK_ERROR
                                   WLF_READ_ERROR
                                   WLF_WAVE_READ_FSEEK_ERROR
                                   WLF_WAVE_READ_ERROR

  Caveats:     The ***wave structure will be allocated by the software.  If you don't have or
               want waveforms, set wave_flag to NVFalse and pass a NULL pointer in the
               ***wave argument (see example in wlf.h).

  *********************************************************************************************/

  WLF_DLL NV_INT32 wlf_read_record (NV_INT32 hnd, NV_INT32 recnum, WLF_RECORD *wlf_record, NV_BOOL wave_flag, NV_INT32 ***wave);





  /*********************************************************************************************

  Function:    wlf_append_record

  Purpose:     Store a WLF record and associated waveforms (if present) at the end of a WLF
               file and update the number_of_records.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle
               wlf_record     -    The WLF record
               wave           -    The waveforms
               shot_num       -    The shot number

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_APPEND_NOT_CREATING_ERROR
                                   WLF_APPEND_FSEEK_ERROR
                                   WLF_WAVE_WRITE_FSEEK_ERROR
                                   WLF_WAVE_WRITE_ERROR
                                   WLF_RECORD_WRITE_FSEEK_ERROR
                                   WLF_RECORD_WRITE_ERROR
                                   WLF_VALUE_OUT_OF_RANGE_ERROR

  Caveats:     The shot number is used to prevent storing redundant waveform data when you
               have multiple returns per shot.  If the shot number doesn't change between
               calls to wlf_append_record then the waveform_address that is stored in the
               first retrun for this shot is stored with all subsequent returns.

  *********************************************************************************************/

  WLF_DLL NV_INT32 wlf_append_record (NV_INT32 hnd, WLF_RECORD wlf_record, NV_INT32 **wave, NV_INT32 shot_num);





  /*********************************************************************************************

  Function:    wlf_update_record

  Purpose:     Update the modifiable fields of a record.  See the WLF_RECORD structure
               definition in wlf.h to see which fields are modifiable after file creation.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle
               recnum         -    The record number
               wlf_record     -    The WLF_RECORD structure to save.

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_UPDATE_RECORD_FSEEK_ERROR
                                   WLF_UPDATE_RECORD_READ_ERROR
                                   WLF_UPDATE_RECORD_WRITE_ERROR

  *********************************************************************************************/

  WLF_DLL NV_INT32 wlf_update_record (NV_INT32 hnd, NV_INT32 recnum, WLF_RECORD wlf_record);





  /*********************************************************************************************

  Function:    wlf_update_header

  Purpose:     Update the modifiable fields of the header record.  See WLF_HEADER in wlf.h to
               determine which fields are modifiable after file creation.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle
               wlf_header     -    The WLF_HEADER structure.

  Returns:     N/A

  *********************************************************************************************/

  WLF_DLL void wlf_update_header (NV_INT32 hnd, WLF_HEADER wlf_header);





  /*********************************************************************************************

  Function:    wlf_close_file

  Purpose:     Close a WLF file.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_CLOSE_FSEEK_ERROR
                                   WLF_CLOSE_WAVE_FSEEK_ERROR
                                   WLF_CLOSE_WAVE_READ_ERROR
                                   WLF_CLOSE_WAVE_WRITE_ERROR
                                   WLF_HEADER_WRITE_FSEEK_ERROR
                                   WLF_HEADER_WRITE_ERROR
                                   WLF_CLOSE_ERROR

  *********************************************************************************************/

  WLF_DLL NV_INT32 wlf_close_file (NV_INT32 hnd);





  /*********************************************************************************************

  Function:    wlf_close_file_ratio

  Purpose:     Special version of wlf_close file that returns the final waveform compression
               ratio.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle
               ratio          -    If the file was just created and had waveforms, this value
                                   will be set to the final compression ratio.  Otherwise, it 
                                   is set to 0.0.
               total_bytes    -    Total number of bytes used for waveforms.

  Returns:     NV_INT32       -    WLF_SUCCESS
                                   WLF_CLOSE_FSEEK_ERROR
                                   WLF_CLOSE_WAVE_FSEEK_ERROR
                                   WLF_CLOSE_WAVE_READ_ERROR
                                   WLF_CLOSE_WAVE_WRITE_ERROR
                                   WLF_HEADER_WRITE_FSEEK_ERROR
                                   WLF_HEADER_WRITE_ERROR
                                   WLF_CLOSE_ERROR

  Caveats:     The ratio returned is based on the min and max values for each waveform type not
               on the original storage used.  In other words, if you had a min and max of 0 and
               1023 (i.e. 10 bits), the compression ration would be based on that, not on the
               fact that you probably had it stored in a 2 byte (i.e. 16 bits) record.  If you
               would like to know that ratio you can use the returned total_bytes field to
               compute it.

  *********************************************************************************************/

  WLF_DLL NV_INT32 wlf_close_file_ratio (NV_INT32 hnd, NV_FLOAT32 *ratio, NV_INT32 *total_bytes);





  /*********************************************************************************************

  Function:    wlf_get_errno

  Purpose:     Returns the latest WLF error condition code

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   N/A

  Returns:     NV_INT32       -    error condition code

  Caveats:     The only thing this is good for at present is to determine if, when you opened
               the file, the library version was older than the file.  That is, if
               WLF_NEWER_FILE_VERSION_WARNING has been set when you called wlf_open_file.
               Otherwise, you can just use wlf_perror or wlf_strerror to get the last
               error information.

  *********************************************************************************************/

  WLF_DLL NV_INT32 wlf_get_errno ();





  /*********************************************************************************************

  Function:    wlf_strerror

  Purpose:     Returns the error string related to the latest error.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   N/A

  Returns:     NV_CHAR       -    Error message

  *********************************************************************************************/

  WLF_DLL NV_CHAR *wlf_strerror ();





  /*********************************************************************************************

  Function:    wlf_perror

  Purpose:     Prints (to stderr) the latest error messages.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   N/A

  Returns:     N/A

  *********************************************************************************************/

  WLF_DLL void wlf_perror ();





  /*********************************************************************************************

  Function:    wlf_get_version

  Purpose:     Returns the WLF library version string

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   N/A

  Returns:     NV_CHAR       -    version string

  *********************************************************************************************/

  WLF_DLL NV_CHAR *wlf_get_version ();





  /*********************************************************************************************

  Function:    wlf_dump_record

  Purpose:     Print the WLF record to stdout.  If you don't want to dump the waveforms just
               send NULL as the **wave address.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/25/09

  Arguments:   hnd            -    The file handle
               wlf_record     -    The WLF record
               wave           -    Pointer to waveforms or NULL

  Returns:     N/A

  *********************************************************************************************/

  WLF_DLL void wlf_dump_record (NV_INT32 hnd, WLF_RECORD wlf_record, NV_INT32 **wave);





  /*  Utility function declarations.  */




  /*********************************************************************************************

    Function        wlf_bit_pack - Packs a long value into consecutive bits in buffer.

    Synopsis        wlf_bit_pack (buffer, start, numbits, value);

                    NV_U_BYTE buffer[]      address of buffer to use
                    NV_U_INT32 start        start bit position in buffer
                    NV_U_INT32 numbits      number of bits to store
                    NV_INT32 value          value to store

    Description     Packs the value 'value' into 'numbits' bits in 'buffer' starting at bit
                    position 'start'.  The majority of this code is based on Appendix C of
                    Naval Ocean Research and Development Activity Report #236, 'Data Base
                    Structure to Support the Production of the Digital Bathymetric Data Base',
                    Nov. 1989, James E. Braud, John L. Breckenridge, James E. Current, Jerry L.
                    Landrum.

    Returns         void

    Author          Jan C. Depner

  *********************************************************************************************/
 
  WLF_DLL void wlf_bit_pack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits, NV_INT32 value);





  /*********************************************************************************************

    Function        bit_unpack - Unpacks a long value from consecutive bits in buffer.

    Synopsis        bit_unpack (buffer, start, numbits);

                    NV_U_BYTE buffer[]      address of buffer to use
                    NV_U_INT32 start        start bit position in buffer
                    NV_U_INT32 numbits      number of bits to retrieve

    Description     Unpacks the value from 'numbits' bits in 'buffer' starting at bit position
                    'start'.  The value is assumed to be unsigned.  The majority of this code
                    is based on Appendix C of Naval Ocean Research and Development Activity
                    Report #236, 'Data Base Structure to Support the Production of the
                    Digital Bathymetric Data Base', Nov. 1989, James E. Braud, John L.
                    Breckenridge, James E. Current, Jerry L. Landrum.

    Returns         NV_U_INT32              value retrieved from buffer

    Author          Jan C. Depner

  *********************************************************************************************/
 
  WLF_DLL NV_U_INT32 wlf_bit_unpack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits);





  /*********************************************************************************************

    Function        wlf_double_bit_pack - Packs a long long integer value into consecutive
                    bits in buffer.

    Synopsis        wlf_double_bit_pack (buffer, start, numbits, value);

                    NV_U_BYTE buffer[]      address of buffer to use
                    NV_U_INT32 start        start bit position in buffer
                    NV_U_INT32 numbits      number of bits to store
                    NV_INT64 value          value to store

    Description     Packs the value 'value' into 'numbits' bits in 'buffer' starting at bit
                    position 'start'.

    Returns         void

    Author          Jan C. Depner

  *********************************************************************************************/
 
  WLF_DLL void wlf_double_bit_pack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits, NV_INT64 value);





  /*********************************************************************************************

    Function        wlf_double_bit_unpack - Unpacks a long long integer value from consecutive
                    bits in buffer.

    Synopsis        wlf_double_bit_unpack (buffer, start, numbits);

                    NV_U_BYTE buffer[]      address of buffer to use
                    NV_U_INT32 start        start bit position in buffer
                    NV_U_INT32 numbits      number of bits to store

    Description     Unpacks a value from 'numbits' bits in 'buffer' starting at bit position
                    'start'.

    Returns         NV_U_INT64              Value unpacked from buffer

    Author          Jan C. Depner

  *********************************************************************************************/
 
  WLF_DLL NV_U_INT64 wlf_double_bit_unpack (NV_U_BYTE buffer[], NV_U_INT32 start, NV_U_INT32 numbits);





  /*********************************************************************************************

    Function        wlf_cvtime - Convert from POSIX time to year, day of year, hour, minute,
                    second.

    Synopsis        wlf_cvtime (tv_sec, tv_nsec, &year, &jday, &hour, &minute, &second);

                    time_t tv_sec           POSIX seconds from epoch (Jan. 1, 1970)
                    long tv_nsec            POSIX nanoseconds of second
                    NV_INT32 year           4 digit year - 1900
                    NV_INT32 jday           day of year
                    NV_INT32 hour           hour of day
                    NV_INT32 minute         minute of hour
                    NV_FLOAT32 second       seconds of minute

    Returns         N/A

    Author          Jan C. Depner

    Caveats         The localtime function will return the year as a 2 digit year (offset
                    from 1900).

  *********************************************************************************************/
 
  WLF_DLL void wlf_cvtime (time_t tv_sec, long tv_nsec, NV_INT32 *year, NV_INT32 *jday, NV_INT32 *hour,
                           NV_INT32 *minute, NV_FLOAT32 *second);





  /*********************************************************************************************

    Function        wlf_inv_cvtime - Convert from year, day of year, hour, minute, second to
                    POSIX time.

    Synopsis        wlf_inv_cvtime (year, jday, hour, minute, second, &tv_sec, &tv_nsec);

                    NV_INT32 year           4 digit year - 1900
                    NV_INT32 jday           day of year
                    NV_INT32 hour           hour of day
                    NV_INT32 minute         minute of hour
                    NV_FLOAT32 second       seconds of minute
                    time_t tv_sec           POSIX seconds from epoch (Jan. 1, 1970)
                    long tv_nsec            POSIX nanoseconds of second

    Returns         N/A

    Author          Jan C. Depner

    Caveats         The mktime function wants a 2 digit year (offset from 1900).

  *********************************************************************************************/
 
  WLF_DLL void wlf_inv_cvtime (NV_INT32 year, NV_INT32 jday, NV_INT32 hour, NV_INT32 min, NV_FLOAT32 sec,
                               time_t *tv_sec, long *tv_nsec);





  /*********************************************************************************************

    Function        wlf_jday2mday - Convert from day of year to month and day.

    Synopsis        wlf_jday2mday (year, jday, &mon, &mday);

                    NV_INT32 year           4 digit year or offset from 1900
                    NV_INT32 jday           day of year
                    NV_INT32 mon            month
                    NV_INT32 mday           day of month

    Returns         N/A

    Author          Jan C. Depner

    Caveats         The returned month value will start at 0 for January.

  *********************************************************************************************/
 
  WLF_DLL void wlf_jday2mday (NV_INT32 year, NV_INT32 jday, NV_INT32 *mon, NV_INT32 *mday);





  /*********************************************************************************************

    Function        wlf_mday2jday - Given year, month, and day, return day of year.

    Synopsis        wlf_mday2jday (year, mon. mday, &jday);

                    NV_INT32 year           4 digit year or offset from 1900
                    NV_INT32 mon            month
                    NV_INT32 mday           day of month
                    NV_INT32 jday           day of year

    Returns         N/A

    Author          Jan C. Depner

    Caveats         The month value must start at 0 for January.

  *********************************************************************************************/
 
  WLF_DLL void wlf_mday2jday (NV_INT32 year, NV_INT32 mon, NV_INT32 mday, NV_INT32 *jday);





  /*********************************************************************************************

    Module Name:        wlf_ngets

    Programmer(s):      Jan C. Depner

    Date Written:       May 1999

    Purpose:            This is an implementation of fgets that strips the line feed (or
                        carriage return/line feed) off of the end of the string if present.

    Arguments:          See fgets

    Return Value:       See fgets

  *********************************************************************************************/

  WLF_DLL NV_CHAR *wlf_ngets (NV_CHAR *s, NV_INT32 size, FILE *stream);



#ifdef  __cplusplus
}
#endif

#endif
