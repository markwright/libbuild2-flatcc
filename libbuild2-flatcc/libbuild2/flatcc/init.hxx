#pragma once

#include <libbuild2/types.hxx>
#include <libbuild2/utility.hxx>

#include <libbuild2/module.hxx>

#include <libbuild2/flatcc/export.hxx>

namespace build2
{
  namespace flatcc
  {
    extern "C" LIBBUILD2_FLATCC_SYMEXPORT const module_functions*
    build2_flatcc_load ();
  }
}
