// file      : libbuild2/utility.ixx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#include <cstring> // strlen() strchr()

namespace build2
{
  namespace flatcc
  {
    template <typename T>
    inline bool
    gen_common_builder (T& s)
    {
      return (find_option ("-a", s, "flatcc.options") ||
              find_option ("-c", s, "flatcc.options") ||
              find_option ("--common", s, "flatcc.options") ||
              find_option ("--common_builder", s, "flatcc.options"));
    }

    template <typename T>
    inline bool
    gen_common_reader (T& s)
    {
      return (find_option ("-a", s, "flatcc.options") ||
              find_option ("-c", s, "flatcc.options") ||
              find_option ("--common", s, "flatcc.options") ||
              find_option ("--common_reader", s, "flatcc.options"));
    }

    template <typename T>
    inline bool
    gen_builder (T& s)
    {
      return (find_option ("-a", s, "flatcc.options") ||
              find_option ("-w", s, "flatcc.options") ||
              find_option ("--builder", s, "flatcc.options"));
    }

    template <typename T>
    inline bool
    gen_reader (T& s)
    {
      if (find_option ("-a", s, "flatcc.options") ||
          find_option ("--reader", s, "flatcc.options"))
        return true;
      else if (find_option ("--json", s, "flatcc.options") ||
               find_option ("--json-parser", s, "flatcc.options") ||
               find_option ("--json-printer", s, "flatcc.options") ||
               find_option ("--schema", s, "flatcc.options"))
        return false;
      else
        return true;
    }

    template <typename T>
    inline bool
    gen_verifier (T& s)
    {
      return (find_option ("-a", s, "flatcc.options") ||
              find_option ("--verifier", s, "flatcc.options"));
    }

    template <typename T>
    inline bool
    gen_json_parser (T& s)
    {
      return (find_option ("--json", s, "flatcc.options") ||
              find_option ("--json-parser", s, "flatcc.options"));
    }

    template <typename T>
    inline bool
    gen_json_printer (T& s)
    {
      return (find_option ("--json", s, "flatcc.options") ||
              find_option ("--json-printer", s, "flatcc.options"));
    }
  }
}

