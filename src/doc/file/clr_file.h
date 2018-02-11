// Aseprite Document Library
// Copyright (c) 2001-2018 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef DOC_FILE_CLR_FILE_H_INCLUDED
#define DOC_FILE_CLR_FILE_H_INCLUDED
#pragma once

namespace doc {

  class Palette;

  namespace file {

    Palette* load_clr_file(const char* filename);
    bool save_clr_file(const Palette* pal, const char* filename);

  } // namespace file

} // namespace doc

#endif
