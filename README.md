Leaklog
=======

Leaklog is a leakage control system based on Regulation (EC) No 842/2006.
It logs findings and parameters of leakage checks, shows a history of the
development of parameters, compares them with nominal ones and calculates
the percentage of leakage.

### Supported platforms

* macOS
* Windows
* Linux and various UNIX-based platforms

### Languages

* Czech (thanks to the *[Czech Association for Cooling and Air Conditioning Technology][chlazeni]*)
* English
* Polish (thanks to *[PROZON Fundacja Ochrony Klimatu][PROZON]*)
* Serbian (thanks to Srđan Đokić)
* Slovak

[chlazeni]: http://www.chlazeni.cz
[PROZON]: http://prozon.org.pl

Installation
------------

Binaries are available for **macOS** and **Windows** on
[GitHub][GH].

[GH]: http://github.com/szchkt/leaklog/releases

To build Leaklog from source, you will need Qt 5.9 or a newer,
compatible version.

Use the `lrelease` tool to compile translations as follows:

	lrelease Leaklog.pro
	lrelease rc/i18n/Leaklog-i18n.ts

If you're running macOS, use:

	qmake -spec macx-clang -config release

Otherwise, the following will do:

	qmake -config release

And finally:

	make
