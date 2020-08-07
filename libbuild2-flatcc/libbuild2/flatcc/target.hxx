// file      : build2/flatcc/target.hxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#ifndef BUILD2_FLATCC_TARGET_HXX
#define BUILD2_FLATCC_TARGET_HXX

#include <libbuild2/types.hxx>
#include <libbuild2/utility.hxx>

#include <libbuild2/target.hxx>

#include <libbuild2/c/target.hxx>

namespace build2
{
  namespace flatcc
  {
    class fbs: public file
    {
    public:
      using file::file;

    public:
      static const target_type static_type;
      virtual const target_type& dynamic_type () const {return static_type;}
    };

    // Standard layout type compatible with group_view's const target*[7].
    //
    struct flatcc_c_members
    {
      const c::h* h_flatbuffers_common_builder = nullptr;
      const c::h* h_flatbuffers_common_reader = nullptr;
      const c::h* h_builder = nullptr;
      const c::h* h_reader = nullptr;
      const c::h* h_verifier = nullptr;
      const c::h* h_json_parser = nullptr;
      const c::h* h_json_printer = nullptr;
    };

    class flatcc_c: public mtime_target, public flatcc_c_members
    {
    public:
      using mtime_target::mtime_target;

      virtual group_view
      group_members (action) const override;

    public:
      static const target_type static_type;

      virtual const target_type&
      dynamic_type () const override {return static_type;}
    };
  }
}

#endif // BUILD2_FLATCC_TARGET_HXX
