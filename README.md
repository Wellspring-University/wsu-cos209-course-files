# WSU-COS 209: The Circuit-to-Cloud Workshop (course files)

Faculty of Computing, Wellspring University · Version 1.0 (October 2026)

These are the files you need for the WSU-COS 209 practicals. Use them with the Practical Manual.

## Getting the files

Download them as a ZIP (green **Code** button, then **Download ZIP**) or, once you have Git installed:

```bash
git clone <this repository's URL>
```

## What is here

| Folder | Contents |
|---|---|
| `arduino/` | Uno sketches for Week 1 and Practicals 1 to 9. Sketches marked *starter* have `TODO` comments for you to finish. See `arduino/README.md` for the wiring of each. |
| `logisim/` | Test-vector files to check your Logisim circuits (Simulate > Test Vector). `logisim/README.md` gives the exact pin labels to use. |
| `web/` | Starter pages for your Lab Portfolio and for Practicals 2 to 9. |
| `esp32/` | ESP32 sketches for Practicals 10 to 12. Copy `secrets.example.h` to `secrets.h` and edit the copy. |
| `server/` | Starter Flask and Node.js servers, the shared dashboard, and the 18 contract tests for Practical 12. |
| `tools/` | `serve.py` (local web server, port 8000) and `check_setup.py`. |

## Before Practical 1

Follow the manual's Preliminary section, then:

```bash
python tools/check_setup.py        # must end with ALL CHECKS PASSED
```

## Keep your secrets out of Git

`secrets.h` (Wi-Fi password, API key) is listed in `.gitignore`. Never commit it, paste it into chat or show it in a screenshot.

## Copyright

Copyright © 2026 O. Adeyeye and [ENTER IFT 203 CO-AUTHOR]. All rights reserved. Registered WSU-COS 209 students may use these files for their course work. See `NOTICE.txt`.
