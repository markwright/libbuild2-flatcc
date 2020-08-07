// file      : build2/flatcc/rule.hxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#ifndef BUILD2_FLATCC_RULE_HXX
#define BUILD2_FLATCC_RULE_HXX

#include <libbuild2/types.hxx>
#include <libbuild2/utility.hxx>

#include <libbuild2/rule.hxx>

namespace build2
{
  namespace flatcc
  {
    // Cached data shared between rules and the module.
    //
    struct data
    {
      const exe&    ctgt; // FLATCC compiler target.
      const string& csum; // FLATCC compiler checksum.
    };

    // @@ Redo as two separate rules?
    //
    class compile_rule: public simple_rule, virtual data
    {
    public:
      compile_rule (data&& d): data (move (d)) {}

      virtual bool
      match (action, target&, const string&) const override;

      virtual recipe
      apply (action, target&) const override;

      target_state
      perform_update (action, const target&) const;
    };

    template <typename T>
    inline bool
    gen_common_builder (T& s);

    template <typename T>
    inline bool
    gen_common_reader (T& s);

    template <typename T>
    inline bool
    gen_builder (T& s);

    template <typename T>
    inline bool
    gen_reader (T& s);

    template <typename T>
    inline bool
    gen_verifier (T& s);

    template <typename T>
    inline bool
    gen_json_parser (T& s);

    template <typename T>
    inline bool
    gen_json_printer (T& s);
  }
}

#include <libbuild2/flatcc/rule.ixx>

#endif // BUILD2_FLATCC_RULE_HXX
