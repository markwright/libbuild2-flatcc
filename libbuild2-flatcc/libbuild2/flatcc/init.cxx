#include <libbuild2/flatcc/init.hxx>

#include <libbuild2/diagnostics.hxx>

using namespace std;

namespace build2
{
  namespace flatcc
  {
    bool
    init (scope&,
          scope&,
          const location& l,
          bool,
          bool,
          module_init_extra&)
    {
      info (l) << "module flatcc initialized";
      return true;
    }

    static const module_functions mod_functions[] =
    {
      {"flatcc", nullptr, init},
      {nullptr, nullptr, nullptr}
    };

    const module_functions*
    build2_flatcc_load ()
    {
      return mod_functions;
    }
  }
}
