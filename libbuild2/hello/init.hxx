#pragma once

#include <libbuild2/types.hxx>
#include <libbuild2/utility.hxx>

#include <libbuild2/module.hxx>

#include <libbuild2/hello/export.hxx>

namespace build2
{
  namespace hello
  {
    extern "C" LIBBUILD2_HELLO_SYMEXPORT const module_functions*
    build2_hello_load ();
  }
}
