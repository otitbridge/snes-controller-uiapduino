# Firmware distribution notices

The firmware combines this project's MIT-licensed application with UIAPduino
Arduino core 1.2.15 (commit `9471a150c9837c9055ef62ef759a1a593eddf1b1`)
and its bundled third-party code. The core's top-level MIT license does not
replace the individual source file licenses.

The release ZIP includes:

- `LICENSE`: Copyright (c) 2026 otitbridge, MIT.
- `licenses/UIAPduino-core-MIT.txt`: Copyright (c) 2026 Tarosa, MIT.
- `licenses/rv003usb-MIT.txt`: Copyright (c) 2023 CNLohr, MIT.
- `licenses/WCH-BSD-3-Clause.txt`: WCH's BSD 3-Clause terms.
- `licenses/upstream-source-notices.txt`: original copyright/license comments
  from the upstream sources, including Arduino contributors, STMicroelectronics,
  WCH, Ha Thach, Daniel Fekete, Charles Lohr, Rich Felker et al. and Michal Ludvig.
- `licenses/LGPL-2.1.txt`: terms for Arduino-derived library code marked
  LGPL-2.1-or-later in the upstream source.
- `licenses/GPL-3.0.txt` and `licenses/GCC-RUNTIME-EXCEPTION-3.1.txt`: GCC runtime
  library licensing. This build uses xPack RISC-V GCC 14.2.0-2.

Complete application source, build scripts, and the core modifications are in
`source/project-source.tar.gz`. The full unmodified core, retaining all original
file notices, is in `source/arduino_core_ch32-1.2.15.zip`. The project contains the
patches to `uiapusb.c`, `uiapusb.h`, and `usb_config.h`, and scripts to apply them
and rebuild the firmware. The source commit and pinned tool versions are recorded
in `manifest.json`. Users may modify and rebuild the library and application;
this distribution does not prohibit reverse engineering for debugging such
library modifications.

Sources:

- https://github.com/tarosay/arduino_core_ch32/tree/9471a150c9837c9055ef62ef759a1a593eddf1b1
- https://github.com/cnlohr/rv003usb
- https://gcc.gnu.org/onlinedocs/libstdc++/manual/license.html

The `uiapflash` executable is not redistributed in this ZIP. The setup helper
downloads the official 1.0.0 archive, verifies it, and retains its LICENSE and
README alongside the installed executable.
