Log window titles when they change
===================================

awitt is a simple window title logger that can be used as a time tracker.
It works quite similar to arbtt (albeit much simpler and more bare-bones),
which means that it needs no manual entry of activities, but instead
relies just on logging window class, title and time spent in the window.

The most simple usage is simply piping the output into a file:

	$ awitt >>activities.log
	$ # work
	$ cat activities.log
	1567425808:22:st-256color:vis /home/niplav/proj/awitt
	1567425822:8:st-256color:awitt /home/niplav/proj/awitt
	1567425868:46:st-256color:vis /home/niplav/proj/awitt
	1567425869:1:st-256color:fish /home/niplav/proj/books/naive_set_theory
	1567425870:1:MuPDF:exercises.pdf - 6/6 (120 dpi)
	1567425881:11:MuPDF:naive_set_theory_halmos_1960.pdf - 33/115 (120 dpi)
	1567425884:3:MuPDF:exercises.pdf - 6/6 (120 dpi)
	1567425887:3:st-256color:vis /home/niplav/proj/books/naive_set_theory
	1567425908:21:MuPDF:naive_set_theory_halmos_1960.pdf - 33/115 (120 dpi)
	1567425909:1:st-256color:vis /home/niplav/proj/awitt

These can then be analysed:

	$ awk -F: '$4~/.*naive_set_theory.*/ { a+=$2 } END { print(a) }' <activities.log
	36

Other use cases could include looking at the activities by time of
day, day of week and so on and maybe even generating visualisations of
the data.

The unix seconds can also (rather awkwardly) be converted to ISO-8601:

	$ awk -F: '{ system("date -Iseconds --date=\"@"$1"\" | tr -d \"\\n\"") | getline d; print(d":"$2":"$3":"$4) }' <activities.log

This often makes them easier to handle, and it is possible that ISO-8601
time will be added in the future.

Requirements
------------

awitt needs the X11 library, and doesn't work with Wayland. It also
(unsurprisingly) needs a C compiler and some standard unix utilities.

Installation
------------

	make install

installs awitt to /usr/local by default.

LICENSE
=======

awitt is licensed with the [MIT LICENSE](./LICENSE).
