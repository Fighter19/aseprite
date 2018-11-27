// Aseprite
// Copyright (C) 2001-2017  David Capello
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.
//
// pcx.c - Based on the code of Shawn Hargreaves.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/file/file.h"
#include "app/file/file_format.h"
#include "app/file/format_options.h"
#include "base/cfile.h"
#include "base/file_handle.h"
#include "doc/doc.h"

#include <Windows.h>

namespace app {

using namespace base;

class SnesFormat : public FileFormat {
  const char* onGetName() const override { return "snes"; }
  const char* onGetExtensions() const override { return "bin,smc,sfc"; }
  dio::FileFormat onGetDioFormat() const override { return dio::FileFormat::SNES_IMAGE; }
  int onGetFlags() const override {
    return
      FILE_SUPPORT_SAVE |
	  FILE_SUPPORT_SEQUENCES |
      FILE_SUPPORT_INDEXED;
  }

  bool onLoad(FileOp* fop) override;

  bool onSave(FileOp* fop) override;

};

FileFormat* CreateSnesFormat()
{
  return new SnesFormat;
}

bool SnesFormat::onLoad(FileOp* fop)
{
	fop->setError("Loading is not supported yet.\n");
	return false;
}

#if 0
bool SnesFormat::onLoad(FileOp* fop)
{
  int c, r, g, b;
  int width, height;
  int bpp, bytes_per_line;
  int xx, po;
  int x, y;
  char ch = 0;

  FileHandle handle(open_file_with_exception(fop->filename(), "rb"));
  FILE* f = handle.get();

  fgetc(f);                    /* skip manufacturer ID */
  fgetc(f);                    /* skip version flag */
  fgetc(f);                    /* skip encoding flag */

  if (fgetc(f) != 8) {         /* we like 8 bit color planes */
    fop->setError("This PCX doesn't have 8 bit color planes.\n");
    return false;
  }

  width = -(fgetw(f));          /* xmin */
  height = -(fgetw(f));         /* ymin */
  width += fgetw(f) + 1;        /* xmax */
  height += fgetw(f) + 1;       /* ymax */

  fgetl(f);                     /* skip DPI values */

  for (c=0; c<16; c++) {        /* read the 16 color palette */
    r = fgetc(f);
    g = fgetc(f);
    b = fgetc(f);
    fop->sequenceSetColor(c, r, g, b);
  }

  fgetc(f);

  bpp = fgetc(f) * 8;          /* how many color planes? */
  if ((bpp != 8) && (bpp != 24)) {
    return false;
  }

  bytes_per_line = fgetw(f);

  for (c=0; c<60; c++)             /* skip some more junk */
    fgetc(f);

  Image* image = fop->sequenceImage(bpp == 8 ?
                                    IMAGE_INDEXED:
                                    IMAGE_RGB,
                                    width, height);
  if (!image) {
    return false;
  }

  if (bpp == 24)
    clear_image(image, rgba(0, 0, 0, 255));

  for (y=0; y<height; y++) {       /* read RLE encoded PCX data */
    x = xx = 0;
    po = rgba_r_shift;

    while (x < bytes_per_line*bpp/8) {
      ch = fgetc(f);
      if ((ch & 0xC0) == 0xC0) {
        c = (ch & 0x3F);
        ch = fgetc(f);
      }
      else
        c = 1;

      if (bpp == 8) {
        while (c--) {
          if (x < image->width())
            put_pixel_fast<IndexedTraits>(image, x, y, ch);

          x++;
        }
      }
      else {
        while (c--) {
          if (xx < image->width())
            put_pixel_fast<RgbTraits>(image, xx, y,
                                      get_pixel_fast<RgbTraits>(image, xx, y) | ((ch & 0xff) << po));

          x++;
          if (x == bytes_per_line) {
            xx = 0;
            po = rgba_g_shift;
          }
          else if (x == bytes_per_line*2) {
            xx = 0;
            po = rgba_b_shift;
          }
          else
            xx++;
        }
      }
    }

    fop->setProgress((float)(y+1) / (float)(height));
    if (fop->isStop())
      break;
  }

  if (!fop->isStop()) {
    if (bpp == 8) {                  /* look for a 256 color palette */
      while ((c = fgetc(f)) != EOF) {
        if (c == 12) {
          for (c=0; c<256; c++) {
            r = fgetc(f);
            g = fgetc(f);
            b = fgetc(f);
            fop->sequenceSetColor(c, r, g, b);
          }
          break;
        }
      }
    }
  }

  if (ferror(f)) {
    fop->setError("Error reading file.\n");
    return false;
  }
  else {
    return true;
  }
}
#endif

#define WRITE8TILE(n) \
runchar = 0;\
for (x = 0+8*xoffset; x<8+8*xoffset; x++) {  /* for each pixel... */\
\
	ch = get_pixel_fast<IndexedTraits>(image, x, y);\
\
	if (x%8 <= (7-n))\
		runchar |= (ch & (1 << n)) << ((7-n) - x%8); /* At least 1 to right*/\
	else\
		runchar |= (ch & (1 << n)) >> (x%8 - (7-n)); /* At least 1 to right*/\
\
}\
fputc(runchar, f);\



bool SnesFormat::onSave(FileOp* fop)
{
  const Image* image = fop->sequenceImage();
  int c, r, g, b;
  int x, y;
  int runcount;
  int depth, planes;
  char runchar;
  volatile char ch = 0;

  FileHandle handle(open_file_with_exception(fop->filename(), "wb"));
  FILE* f = handle.get();

  // image->width height
  // image pixelformat

  // ch = get_pixel_fast<IndexedTraits>(image, x, y);

  if (image->pixelFormat() != IMAGE_INDEXED)
  {
	  fop->setError("SNES files only have indexed mode.\n");
	  return false;
  }


  int yoffset = 0, xoffset = 0;

  for (int k = 0; k < image->height() / 32; k++) {
	  /*One row of 32x32 tiles*/
	  for (int j = 0; j < image->width() / 32; j++)
	  {
		  /*32x32 tile*/
		  for (yoffset = 0 + 4 * k; yoffset < 4 + 4 * k; yoffset++) {
			  for (xoffset = 0 + 4 * j; xoffset < 4 + 4 * j; xoffset++)
			  {
				  for (y = 0 + 8 * yoffset; y < 8 + 8 * yoffset; y++) {           /* for each scanline... */
					  runcount = 0;
					  WRITE8TILE(0)
						  WRITE8TILE(1)


						  fop->setProgress((float)(y + 1) / ((float)(image->height()) * 2));
				  }

				  for (y = 0 + 8 * yoffset; y < 8 + 8 * yoffset; y++) {           /* for each scanline... */
					  runcount = 0;
					  WRITE8TILE(2)
						  WRITE8TILE(3)

						  fop->setProgress((float)(y + 1) / ((float)(image->height()) * 2));
				  }
			  }
		  }
	  }
  }


  if (depth == 8) {                      /* 256 color palette */
    // Save palette into other file
  }

  if (ferror(f)) {
    fop->setError("Error writing file.\n");
    return false;
  }
  else {
    return true;
  }
}




} // namespace app
