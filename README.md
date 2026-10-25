Magic Pockets (The Bitmap Brothers, 1991; Archimedes conversion, Renegade, 1993) - static recompilation
====================================================================================================

A recipe: this repository holds no game data. It rebuilds the game as a
native executable from your own copy, recognised by content (recipe.json),
so it can be shared without distributing the game.

    ./build.sh "Magic Pockets (1993) (Renegade).zip"
    cd recomp && ./pockets

build.sh runs recipe.json's steps with SuperSpider's tools/weave.py
(~/stuff/superspider, or $SUPERSPIDER; $SS below): it extracts the disc,
unpacks the two programs with Loader's own decompressor (tools/unpack.py,
about two minutes in Python), analyses and translates them, and links them
with libroger (~/stuff/libroger, or make ROGER=...). Needs python3,
OCaml/dune, gcc and SDL3.

The input: the zip of an Arculator .apd ("Magic Pockets (1993)
(Renegade).apd", a flux-level dump, and its .log). $SS/tools/apd.py decodes
it to sectors: an ADFS E disc holding !Pockets, plus a copy protection
track (track 79 side 0: four 512-byte sectors numbered from 240, the ID of
one hidden inside another's data, two with bad CRCs). Tracks 52-55 side 1
have a sector with a bad CRC each, in no file.

What it makes:

    extracted/disc       the disc's files, via weave.py
    extracted/disc.apd   the disc itself, gunzipped (the recipe's "raw"), for
                         the protection check: libroger reads it as the floppy
    game/!Pockets        libroger's appdir (../game)
    unpacked/intro, pockets   tools/unpack.py: the programs as Loader unpacks them
    analysis/*.json      SuperSpider's analyses; loader.roots: an entry point it can't find
    recomp/gen*/         the C translations
    recomp/pockets       the game

Committed: recipe.json, build.sh, tools/unpack.py, analysis/loader.roots,
recomp/Makefile, recomp/game.c, recomp/pockets.ini, this file.

Playing
-------

    ./pockets               as !Run: the intro (Renegade, the Bitmap Brothers, the
                            title, with music), then the demo, and round again
    ./pockets --crop        less letterboxing on a widescreen monitor: rows 0-207
                            (recomp/pockets.ini)

Keys: Z left, X right, ' up (jump), / down, Return fire (the game's
defaults; its own key table at &35F81). Return in the intro starts a game.
In a game, Escape abandons it and Ctrl-Escape quits (OS_Exit: the run
ends); F2-F5 call routines of the game's own (&E65C, &E644, &E3D4, &DDA8:
not identified). A game controller stands in for Acorn's
joystick interface (Joystick_Read, which the game reads when a controller is
connected, or with --joystick): the stick or D-pad moves, A fires.

How the game is put together
----------------------------

!Run sets Pockets$Dir, Vga$Mode 0 and Demo$Mode 2, runs FreeMem (a utility
that frees the RAM disc, font cache and so on and gives the screen 80K: as
libroger starts out, so it isn't run), RMLoads PM (TrackerModule 4.09: a
different build from DinoSaw's), sets a 640K WimpSlot, selects MODE 9 and
runs Loader.

- Loader (&707E0-&710C4, an absolute): checks for 80K of screen memory,
  reads Demo$Mode and, with bit 1 set, loads Intro at &8000, otherwise
  Pockets. Both are AIF images packed with Rob Northern's ProPack (RNC
  method 1); &70950 unpacks the file in place, and Loader enters Intro at
  &8088 or Pockets at &A614. Loader stays in memory above them, and each
  program goes back to it (&707FC) to load the other: the intro sets
  Demo$Mode 0 (Return pressed: a game) or 1 (the demo) and the game, after
  its demo, 2.
- Intro (&8000-&70584 unpacked; code &8000-&8CFC, the rest pictures and
  the tune): the Renegade and Bitmap Brothers screens and the title, with
  music through TrackerModule.
- Pockets (&8000-&3608C unpacked): the game. Its workspace is in R5. It
  starts a game with a disc check: &D948-&DB2C is encrypted in the image
  and decrypted in place before it's called. It sets ADFS_Retries and reads
  sectors 240 and 242 of track 79 side 0 (ADFS_DiscOp with a disc record of
  its own: 250 sectors of 512 bytes a track), checksums them, and after 12
  tries asks for the original disc in drive 0. The demo doesn't check.

What's translated (recomp/Makefile, make regen): Loader (genload/, the
program), Pockets (gen/) and Intro (genintro/), both overlays at &8000, and
TrackerModule (gentracker/, at &1800000). Each program's analysis takes the
other's range as --extern. Loader's re-entry at &707FC is a root
(analysis/loader.roots). The disc check runs interpreted, as it is once
decrypted (roger_config.interpreted), with the dump as the floppy disc.

Tested headless: two full rounds of intro and demo (1500M instructions:
the music, the demo's sound effects), and games started from the intro (the
disc check, the map, the cave), walking, jumping and firing, reach no
untranslated code. Later levels, dying, game over, the high scores and the
ending weren't reached in testing; their code is translated as far as the
analysis finds it (36664 words of Pockets; 43 jumps it couldn't resolve are
dispatched at run time).

What SuperSpider and libroger needed
------------------------------------

SuperSpider's tools:

- apd.py (new): Arculator's .apd images to sectors; weave.py takes them as
  it takes .hfe and .jfd (a recipe's "raw" copy is written gunzipped, for
  libroger), and zfiler.py reads their sectors too.
- hfe.py (used by apd.py): a sector's data field is no longer skipped when
  looking for the next ID, as a 1772 controller doesn't skip it: sector
  242's ID is inside 241's data. The sectors decoded from every other .hfe
  dump (Darkwood's six, James Pond's, James Pond 2's) are unchanged.

libroger:

- roger_program.unpacked: a program loaded packed over its own image and
  unpacked there (Intro and Pockets). Its code words are compared with the
  image as unpacked, not with the packed bytes that the unpacking stores find
  there, which kept the unpacked code interpreted for good. As the packed
  file loads, its words that don't hold their unpacked values yet are marked
  dirty.
- The dispatcher interprets a dirty word that's back as translated but
  isn't an entry of the translation (translated code leaves at such a word,
  part way through a function).
- The floppy can be an .apd (uncompressed): its MFM tracks decoded as an
  .hfe's are. As in hfe.py, a sector's data field is no longer skipped when
  looking for IDs.

Regression: every other game (32) built against libroger before and after
these changes, and its headless run (100M instructions) compared: the event
log, VDU stream, screen and registers are byte-identical in all of them.
SuperSpider's analysis and C generation weren't changed.

Not supported: the VGA version (Vga$Mode 1: the MyMode module's mode and
FreeMem2's larger screen).
