#include <libbuild2/flatcc/init.hxx>

#include <libbuild2/file.hxx>
#include <libbuild2/scope.hxx>
#include <libbuild2/target.hxx>
#include <libbuild2/variable.hxx>
#include <libbuild2/diagnostics.hxx>

#include <libbuild2/config/utility.hxx>

#include <libbuild2/c/target.hxx>

#include <libbuild2/flatcc/rule.hxx>
#include <libbuild2/flatcc/module.hxx>
#include <libbuild2/flatcc/target.hxx>

using namespace std;

namespace build2
{
  namespace flatcc
  {
    // Remaining issues/semantics change:
    //
    // @@ Unconfigured caching.
    //
    // @@ Default-found flatcc used to result in config.flatcc=flatcc and now it's just
    //    omitted (and default-not-found -- in config.flatcc.configured=false).
    //
    //    - Writing any default will take precedence over config.import.flatcc.
    //      In fact, this duality is a bigger problem: if we have a config
    //      that uses config.flatcc there is no way to reconfigure it to use
    //      config.import.flatcc.
    //
    //    - We could have saved it commented.
    //
    //    - We could do this at the module level only since we also have
    //      config.flatcc.options?
    //
    //    - Note that in the FLATCC compiler itself we now rely on default flatcc
    //      being NULL/undefined. So if faving, should probably be commented
    //      out. BUT: it will still be defined, so will need to be defined
    //      NULL. Note also that long term the FLATCC compiler will not use the
    //      module relying on an ad hoc recipe instead.
    //
    //    ! Maybe reserving NULL (instead of making it the same as NULL) for
    //      this "configured to default" state and saving commented is not a
    //      bad idea. Feels right to have some marker in config.build that
    //      things are in effect. And I believe if config.import.flatcc is
    //      specified, it will just be dropped.

    bool
    guess_init (scope& rs,
                scope& bs,
                const location& loc,
                bool,
                bool optional,
                module_init_extra& extra)
    {
      tracer trace ("flatcc::guess_init");
      l5 ([&]{trace << "for rs:" << rs << " bs: " << bs;});

      // We only support root loading (which means there can only be one).
      //
      // if (rs != bs)
      //   fail (loc) << "flatcc.guess module must be loaded in project root";

      // Adjust module config.build save priority (code generator).
      //
      config::save_module (rs, "flatcc", 150);

      // Enter metadata variables.
      //
      auto& vp (rs.var_pool ());

      auto& v_ver (vp.insert<string> ("flatcc.version"));
      auto& v_sum (vp.insert<string> ("flatcc.checksum"));

      // Import the FLATCC compiler target.
      //
      // Note that the special config.flatcc=false value (recognized by the
      // import machinery) is treated as an explicit request to leave the
      // module unconfigured.
      //
      bool new_cfg (false);
      pair<const exe*, import_kind> ir (
        import_direct<exe> (
          new_cfg,
          rs,
          name ("flatcc", dir_path (), "exe", "flatcc"), // flatcc%exe{flatcc}
          true      /* phase2 */,
          optional,
          true      /* metadata */,
          loc,
          "module load"));

      const exe* tgt (ir.first);

      // Extract metadata.
      //
      auto* ver (tgt != nullptr ? &cast<string> (tgt->vars[v_ver]) : nullptr);
      auto* sum (tgt != nullptr ? &cast<string> (tgt->vars[v_sum]) : nullptr);

      // Print the report.
      //
      // If this is a configuration with new values, then print the report
      // at verbosity level 2 and up (-v).
      //
      if (verb >= (new_cfg ? 2 : 3))
      {
        diag_record dr (text);
        dr << "flatcc " << project (rs) << '@' << rs << '\n';

        if (tgt != nullptr)
          dr << "  flatcc        " << ir << '\n'
             << "  version    " << *ver << '\n'
             << "  checksum   " << *sum;
        else
          dr << "  flatcc        " << "not found, leaving unconfigured";
      }

      if (tgt == nullptr)
        return false;

      // The flatcc variable (untyped) is an imported compiler target name.
      //
      rs.assign ("flatcc") = tgt->as_name ();
      rs.assign (v_sum) = *sum;
      rs.assign (v_ver) = *ver;

      {
        standard_version v (*ver);

        rs.assign<uint64_t> ("flatcc.version.number") = v.version;
        rs.assign<uint64_t> ("flatcc.version.major") = v.major ();
        rs.assign<uint64_t> ("flatcc.version.minor") = v.minor ();
        rs.assign<uint64_t> ("flatcc.version.patch") = v.patch ();
      }

      // Cache some values in the module for easier access in the rule.
      //
      extra.set_module (new module (data {*tgt, *sum}));

      return true;
    }

    bool
    config_init (scope& rs,
                 scope& bs,
                 const location& loc,
                 bool,
                 bool optional,
                 module_init_extra& extra)
    {
      tracer trace ("flatcc::config_init");
      l5 ([&]{trace << "for rs: " << rs << " bs: " << bs;});

      // We only support root loading (which means there can only be one).
      //
      // if (rs != bs)
      //    fail (loc) << "flatcc.config module must be loaded in project root";

      // Load flatcc.guess and share its module instance as ours.
      //
      if (const shared_ptr<build2::module>* r = load_module (
            rs, rs, "flatcc.guess", loc, optional, extra.hints))
      {
        extra.module = *r;
      }
      else
      {
        // This can happen if someone already optionally loaded flatcc.guess
        // and it has failed to configure.
        //
        if (!optional)
          fail (loc) << "flatcc could not be configured" <<
            info << "re-run with -V for more information";

        return false;
      }

      // Configuration.
      //
      using config::append_config;

      // config.flatcc.options
      //
      // Note that we merge it into the corresponding flatcc.* variable.
      //
      append_config<strings> (rs, rs, "flatcc.options", nullptr);

      return true;
    }

    bool
    init (scope& rs,
          scope& bs,
          const location& loc,
          bool,
          bool optional,
          module_init_extra& extra)
    {
      tracer trace ("flatcc::init");
      l5 ([&]{trace << "for rs: " << rs << " bs: " << bs;});

      // We only support root loading (which means there can only be one).
      //
      // if (rs != bs)
      //   fail (loc) << "flatcc module must be loaded in project root";

      // Make sure the c module has been loaded since we need its targets
      // types. Note that we don't try to load it ourselves because of
      // the non-trivial variable merging semantics. So it is better to let
      // the user load cxx explicitly. @@ Not sure the reason still holds
      // though it might still make sense to expect the user to load c.
      //
      if (!cast_false<bool> (rs["c.loaded"]))
        fail (loc) << "c module must be loaded before flatcc";

      // Load flatcc.config and get its module instance.
      //
      if (const shared_ptr<build2::module>* r = load_module (
            rs, rs, "flatcc.config", loc, optional, extra.hints))
      {
        extra.module = *r;
      }
      else
      {
        // This can happen if someone already optionally loaded flatcc.config
        // and it has failed to configure.
        //
        if (!optional)
          fail (loc) << "flatcc could not be configured" <<
            info << "re-run with -V for more information";

        return false;
      }

      auto& m (extra.module_as<module> ());

      // Register target types.
      //
      rs.insert_target_type<fbs> ();
      rs.insert_target_type<flatcc_c> ();

      // Register our rules.
      //
      {
        auto reg = [&rs, &m] (meta_operation_id mid, operation_id oid)
        {
          rs.insert_rule<flatcc_c>  (mid, oid, "flatcc.compile", m);
          rs.insert_rule<c::h> (mid, oid, "flatcc.compile", m);
        };

        reg (perform_id, update_id);
        reg (perform_id, clean_id);

        // Other rules (e.g., cc::compile) may need to have the group members
        // resolved/linked up. Looks like a general pattern: groups should
        // resolve on *(update).
        //
        // @@ meta-op wildcard?
        //
        reg (configure_id, update_id);
        reg (dist_id, update_id);
      }

      info (loc) << "module flatcc initialized";
      return true;
    }

    static const module_functions mod_functions[] =
    {
      // NOTE: don't forget to also update the documentation in init.hxx if
      //       changing anything here.

      {"flatcc.guess",  nullptr, guess_init},
      {"flatcc.config", nullptr, config_init},
      {"flatcc",        nullptr, init},
      {nullptr,         nullptr, nullptr}
    };

    const module_functions*
    build2_flatcc_load ()
    {
      return mod_functions;
    }
  }
}
