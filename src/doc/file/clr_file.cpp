// Aseprite Document Library
// Copyright (c) 2001-2018 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/base.h"
#include "base/cfile.h"
#include "base/clamp.h"
#include "doc/color_scales.h"
#include "doc/image.h"
#include "doc/palette.h"

#include <cstdio>
#include <cstdlib>


namespace doc {
namespace file {

using namespace base;

// Loads a CLR file (SNES RGB555 format)
Palette* load_clr_file(const char* filename)
{
  Palette *pal = nullptr;
  int c, r, g, b;
  FILE* f;

  f = std::fopen(filename, "rb");
  if (!f)
    return NULL;

  // Get file size.
  std::fseek(f, 0, SEEK_END);
  std::size_t size = std::ftell(f);
  std::fseek(f, 0, SEEK_SET);

  if (!(size)) {       
    fclose(f);
    return NULL;
  }

    pal = new Palette(frame_t(0), 256);

    for (c=0; c<256; c++) {

	  char col1 = fgetc(f);
	  char col2 = fgetc(f);

	  // First byte is  gggrrrrr
	  // Second byte is 0bbbbbgg
	  r = col1 & 0b00011111;
	  g = ((col1 >> 5) & 0b00000111) | ((col2 << 3) & 0b00011000);
	  b = (col2 & 0b01111100) >> 2;

      if (ferror(f))
        break;

      pal->setEntry(c, rgba(scale_5bits_to_8bits(r),
                            scale_5bits_to_8bits(g),
                            scale_5bits_to_8bits(b), 255));
    
  }

  fclose(f);
  return pal;
}

// Saves a SNES RGB555 file
bool save_clr_file(const Palette* pal, const char* filename)
{
	
  FILE *f = fopen(filename, "wb");
  if (!f)
    return false;

  uint32_t c;
  for (int i=0; i<256; i++) {
    c = pal->getEntry(i);

	int r, g, b;
	r = rgba_getr(c) >> 3;
	g = rgba_getg(c) >> 3;
	b = rgba_getb(c) >> 3;

    fputc((r & 0b00011111) | ((g << 5) & 0b11100000), f);
    fputc((b << 2) & 0b01111100 | ((g >> 3) & 0b00000011), f);
    if (ferror(f))
      break;
  }

  fclose(f);
  
  return true;
}

} // namespace file
} // namespace doc
