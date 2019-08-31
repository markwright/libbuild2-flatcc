#include <libbuild2/hello/init.hxx>

#include <libbuild2/diagnostics.hxx>

using namespace std;

namespace build2
{
  namespace hello
  {
    bool
    init (scope&,
          scope&,
          const location& l,
          unique_ptr<module_base>&,
          bool,
          bool,
          const variable_map&)
    {
      info (l) << "module hello initialized";
    }

    static const module_functions mod_functions[] =
    {
      {"hello", nullptr, init},
      {nullptr, nullptr, nullptr}
    };

    const module_functions*
    build2_hello_load ()
    {
      return mod_functions;
    }
  }
}
