

#include "Arduino.h"

/*
	SETTINGS — example usage.

	Three parts, all in main.cpp:

	i.   BUFFERS.  One char[SETTINGS_BUF_SIZE] per setting.  The initializer
	     is the compiled default.  Name the buffers whatever you like — the
	     cloud never sees these names.

	         char _threshold[SETTINGS_BUF_SIZE]  = "2.5";
	         char _sleep_secs[SETTINGS_BUF_SIZE] = "30";
	         char _mode[SETTINGS_BUF_SIZE]       = "auto";

	     Typed variables for anything you use as a number:

	         float threshold;
	         int   sleep_secs;

	ii.  TABLE.  Maps each dashboard key to its buffer.  Keys: 15 chars max,
	     no '/', '+', '#', or spaces.  End with the { nullptr, nullptr } row.

	         SettingRow settings_table[] = {
	             { "threshold",  _threshold  },
	             { "sleep_secs", _sleep_secs },
	             { "mode",       _mode       },
	             { nullptr,      nullptr     },
	         };

	iii. CONVERSION FUNCTION.  Re-derives every typed variable from its
	     buffer.  No keys, no ifs — one line per numeric setting.  The
	     framework calls it once at boot and after every remote update.
	     Strings need no line: the buffer IS the setting (use _mode directly).

	         void settingsChanged() {
	             threshold  = settings.convert_to_float(_threshold, 2.5);
	             sleep_secs = settings.convert_to_int(_sleep_secs, 30);
	         }

	     The second argument is the fallback if someone sends a value that
	     isn't a number.  The plain one-argument flavor returns 0 instead.
	     Either way the failure is reported to your app's Messages inbox.

	Then ONE call in setup(), after controller.setup():

	         settings.begin(settings_table, settingsChanged);

	That's everything.  The framework loads stored values from flash (or
	persists your defaults on first boot), runs your conversion function,
	shows the settings on the device dashboard, applies dashboard edits,
	persists them, re-runs your function, and echoes the held value back
	so the dashboard always shows the truth.

	Notes:
	  - An edit with an unknown key, or one too long for the buffer, is
	    refused — the dashboard snaps back to the value the device holds.
	  - Want to validate?  Do it in settingsChanged():
	        float t = settings.convert_to_float(_threshold, threshold);
	        if (t >= 0 && t <= 100) threshold = t;   // else keep the old value
	    The buffer (and dashboard) still show what was sent — your float
	    just declines to follow it.
*/