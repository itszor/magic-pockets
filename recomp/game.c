/* Magic Pockets (The Bitmap Brothers, 1991; Archimedes conversion published
 * by Renegade, 1993): what libroger needs to know about the game.
 *
 * !Run sets Pockets$Dir, Vga$Mode 0 (1: a VGA version, for multisync
 * monitors, with MyMode's mode and FreeMem2's larger screen) and Demo$Mode
 * 2 (0 skips the intro), runs FreeMem (a utility that frees the RAM disc,
 * font cache and so on and gives the screen 80K: as libroger starts out,
 * so it isn't run), RMLoads PM (TrackerModule 4.09), sets a 640K
 * WimpSlot, selects MODE 9 (ECHO <22><9>) with a "please wait" message and
 * runs Loader, an absolute at &707E0.
 *
 * Loader checks the screen memory and reads Demo$Mode: with bit 1 set it
 * loads Intro at &8000, otherwise Pockets; both are AIF images packed with
 * Rob Northern's ProPack, which Loader unpacks in place (&70950;
 * ../tools/unpack.py runs that to make the images translated here) and
 * enters (Intro at &8088, Pockets at &A614). Loader stays in memory above
 * them: the intro sets Demo$Mode and goes back to it (&707FC) to load the
 * game, and the game goes back to it for the intro.
 *
 * The translated images: Loader (genload/), the game (gen/), the intro
 * (genintro/), TrackerModule (gentracker/). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <roger.h>

extern const struct roger_image recomp_image, recompI_image, recompL_image, recompM_image;

/* Loader unpacks each program over its packed file at &8000 and enters it
 * part way through: overlays, the last one loaded there dispatched, their
 * code compared with the images as unpacked (../tools/unpack.py) */
static const struct roger_program programs[] = {
    {"Intro", &recompI_image, 1, 0x8000, 0x8088, "../unpacked/intro"},
    {"Pockets", &recomp_image, 1, 0x8000, 0xA614, "../unpacked/pockets"},
    {NULL, NULL, 0, 0, 0, NULL}};

static const struct roger_module modules[] = {
    /* RMEnsure TrackerModule 4.09 RMLoad <Pockets$Dir>.PM */
    {"../game/!Pockets/PM,fffffa44-590bc522", &recompM_image, 0, NULL, 0},
    {NULL, NULL, 0, NULL, 0}};

static const char *const vars[] = {
    "Pockets$Dir", "!Pockets",
    "Vga$Mode", "0",
    "Demo$Mode", "2",
    NULL};

/* The disc check, run as a game starts (not in the demo): &D948-&DB2C is
 * encrypted in the image and decrypted in place before it's called. It
 * reads track 79 side 0's four 512-byte sectors (ADFS_DiscOp: the disc's
 * odd ones, two with bad CRCs), checksums them and, after 12 tries,
 * asks for the original disc in drive 0. It runs interpreted, as it is
 * then; the disc is the dump itself (.floppy). */
static const uint32_t interpreted[] = {0xD948, 0xDB30, 0, 0};

/* The headless scenario (--limit runs; instruction counts, 2000 a ms):
 * Return in the intro skips to a game (the disc check, the map, THE CAVE),
 * then Z walks left, X back right, ' jumps the block there, Return fires */
static const struct roger_key keys[] = {
    {74, 80000000, 80400000},                       /* Return: skip the intro */
    {98, 140000000, 144000000},                     /* Z: left */
    {67, 146000000, 170000000},                     /* X: right */
    {80, 152000000, 153000000},                     /* ': jump */
    {74, 156000000, 156600000},                     /* Return: fire */
    {80, 160000000, 161000000},
    {74, 164000000, 164600000},
};

static struct roger_config config = {
    .title = "Magic Pockets (recompiled)",
    .image_path = "../game/!Pockets/Loader,000707e0-000707e0",
    .image = &recompL_image,
    .entry = 0x707E0,
    .programs = programs,
    .modules = modules,
    .appdir = "../game",
    .variables = vars,
    .command_line = "<Pockets$Dir>.Loader",
    .floppy = "../extracted/disc.apd",  /* (weave.py keeps it, gunzipped) */
    .interpreted = interpreted,
    .initial_mode = 9,                  /* !Run: ECHO <22><9> */
    .wimpslot = 640 * 1024,             /* WimpSlot -min 640k -max 640k */
    /* both programs read Acorn's joystick module (Joystick_Read), which a
     * controller stands in for */
    .joystick = ROGER_JOY_ACORN,
    .keys = keys,
    .nkeys = sizeof keys / sizeof keys[0],
};

int main(int argc, char **argv) { roger_main(argc, argv, &config); }
