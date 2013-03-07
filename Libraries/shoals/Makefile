ARFLAGS=r

CFLAGS = -O -ansi -DUNIX -D_XOPEN_SOURCE

CC=gcc

.c.o:
	$(CC) -c $(CFLAGS) $*.c

libshoals_file.a:	libshoals_file.a(file_abdepth.o) \
			libshoals_file.a(file_diag_flight.o)  \
			libshoals_file.a(file_diag_pulse.o)  \
			libshoals_file.a(file_dr_lplse.o)  \
			libshoals_file.a(file_flightline.o)  \
			libshoals_file.a(file_flightmap.o)  \
			libshoals_file.a(file_globals.o)  \
			libshoals_file.a(file_gps.o)  \
			libshoals_file.a(file_haps.o)  \
			libshoals_file.a(file_hardware.o)  \
			libshoals_file.a(file_input.o)  \
			libshoals_file.a(file_ins.o)  \
			libshoals_file.a(file_otf_zone.o)  \
			libshoals_file.a(file_output.o)  \
			libshoals_file.a(file_res_flight.o)  \
			libshoals_file.a(file_res_pulse.o)  \
			libshoals_file.a(file_survey_limits.o)  \
			libshoals_file.a(file_tide.o)  \
			libshoals_file.a(file_wave.o)

	cp libshoals_file.a $(PFM_LIB)
