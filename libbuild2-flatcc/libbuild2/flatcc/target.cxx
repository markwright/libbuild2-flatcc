// file      : build2/flatcc/target.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#include <libbuild2/flatcc/target.hxx>

#include <libbuild2/context.hxx>

namespace build2
{
  namespace flatcc
  {
    // flatcc
    //
    extern const char flatcc_ext_def[] = "fbs";

    const target_type fbs::static_type
    {
      "fbs",
      &file::static_type,
      &target_factory<fbs>,
      nullptr, /* fixed_extension */
      &target_extension_var<flatcc_ext_def>,
      &target_pattern_var<flatcc_ext_def>,
      nullptr,
      &file_search,
      false
    };

    // fbs.c
    //
    group_view flatcc_c::
    group_members (action) const
    {
      static_assert (sizeof (flatcc_c_members) == sizeof (const target*) * 7,
                     "member layout incompatible with array");

      return h_flatbuffers_common_builder != nullptr
        ? group_view {reinterpret_cast<const target* const*> (&h_flatbuffers_common_builder),
                      7U}
        : group_view {nullptr, 0};
    }

    static target*
    fbs_c_factory (context& ctx,
                   const target_type&, dir_path d, dir_path o, string n)
    {
      tracer trace ("flatcc::flatcc_cxx_factory");

      // Pre-enter (potential) members as targets. The main purpose of doing
      // this is to avoid searching for existing files in src_base if the
      // buildfile mentions some of them explicitly as prerequisites.
      //
      // Also required for the src-out remapping logic.
      //
      ctx.targets.insert<c::h> (d, o, "flatbuffers_common_builder.h", trace);
      ctx.targets.insert<c::h> (d, o, "flatbuffers_common_reader.h", trace);
      ctx.targets.insert<c::h> (d, o, n + "_builder.h", trace);
      ctx.targets.insert<c::h> (d, o, n + "_reader.h", trace);
      ctx.targets.insert<c::h> (d, o, n + "_verifier.h", trace);
      ctx.targets.insert<c::h> (d, o, n + "_json_parser.h", trace);
      ctx.targets.insert<c::h> (d, o, n + "_json_printer.h", trace);

      return new flatcc_c (ctx, move (d), move (o), move (n));
    }

    const target_type flatcc_c::static_type
    {
      "fbs.c",
      &mtime_target::static_type,
      &fbs_c_factory,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      &target_search,
      true // "See through" default iteration mode.
    };
  }
}
