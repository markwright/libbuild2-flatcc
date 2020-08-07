// file      : build2/flatcc/rule.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#include <libbuild2/flatcc/rule.hxx>

#include <libbuild2/depdb.hxx>
#include <libbuild2/scope.hxx>
#include <libbuild2/target.hxx>
#include <libbuild2/context.hxx>
#include <libbuild2/algorithm.hxx>
#include <libbuild2/filesystem.hxx>
#include <libbuild2/diagnostics.hxx>

#include <libbuild2/flatcc/target.hxx>

namespace build2
{
  namespace flatcc
  {
    // Figure out if name contains stem and, optionally, calculate prefix and
    // suffix.
    //
    static bool
    match_stem (const string& name, const string& stem,
                string* prefix = nullptr, string* suffix = nullptr)
    {
      size_t p (name.find (stem));

      if (p != string::npos)
      {
        if (prefix != nullptr)
          prefix->assign (name, 0, p);

        if (suffix != nullptr)
          suffix->assign (name, p + stem.size (), string::npos);

        return true;
      }

      return false;
    }

    bool compile_rule::
    match (action a, target& t, const string&) const
    {
      tracer trace ("flatcc::compile_rule::match");

      // Find the .fbs source file.
      //
      auto find = [&trace, a, &t] (auto&& r) -> optional<prerequisite_member>
      {
        for (prerequisite_member p: r)
        {
          // If excluded or ad hoc, then don't factor it into our tests.
          //
          if (include (a, t, p) != include_type::normal)
            continue;

          if (p.is_a<fbs> ())
          {
            // Check that the stem match.
            //
            if (match_stem (t.name, p.name ()))
              return p;

            l4 ([&]{trace << ".fbs file stem '" << p.name () << "' "
                          << "doesn't match target " << t;});
          }
        }

        return nullopt;
      };

      if (flatcc_c* pt = t.is_a<flatcc_c> ())
      {
        // The flatcc.c{} group.
        //
        flatcc_c& t (*pt);

        // See if we have a .fbs source file.
        //
        if (!find (group_prerequisite_members (a, t)))
        {
          l4 ([&]{trace << "no .fbs source file for target " << t;});
          return false;
        }

        // Figure out the member list.
        //
        // Note: I'm not using the --recursive option.  It seems easier to call
        // flatcc multiple times instead of using the --recursive option.
        t.h_flatbuffers_common_builder =
          gen_common_builder(t)
          ? nullptr
          : &search<c::h> (t, t.dir, t.out,
                           "flatbuffers_common_builder");
        t.h_flatbuffers_common_reader =
          gen_common_reader(t)
          ? nullptr
          : &search<c::h> (t, t.dir, t.out,
                           "flatbuffers_common_reader");
        t.h_builder =
          gen_builder(t)
          ? nullptr
          : &search<c::h> (t, t.dir, t.out, t.name + "_builder");
        t.h_reader =
          gen_reader(t)
          ? nullptr
          : &search<c::h> (t, t.dir, t.out, t.name + "_reader");
        t.h_verifier =
          gen_verifier(t)
          ? nullptr
          : &search<c::h> (t, t.dir, t.out, t.name + "_verifier");
        t.h_json_parser =
          gen_json_parser(t)
          ? nullptr
          : &search<c::h> (t, t.dir, t.out, t.name + "_json_parser");
        t.h_json_printer =
          gen_json_printer(t)
          ? nullptr
          : &search<c::h> (t, t.dir, t.out, t.name + "_json_printer");
        return true;
      }
      else
      {
        // One of the h{} members.
        //

        // Check if there is a corresponding flatcc.c{} group.
        //
        const flatcc_c* g (t.ctx.targets.find<flatcc_c> (t.dir, t.out, t.name));

        // If not or if it has no prerequisites (happens when we use it to
        // set flatcc.options) and this target has a flatcc{} prerequisite, then
        // synthesize the dependency.
        //
        if (g == nullptr || !g->has_prerequisites ())
        {
          if (optional<prerequisite_member> p = find (
                prerequisite_members (a, t)))
          {
            if (g == nullptr)
              g = &t.ctx.targets.insert<flatcc_c> (t.dir, t.out, t.name, trace);

            g->prerequisites (prerequisites {p->as_prerequisite ()});
          }
        }

        if (g == nullptr)
          return false;

        if (match_stem(t.name, "common_builder.h") && !gen_common_builder(*g))
          return false;

        if (match_stem(t.name, "common_reader.h") && !gen_common_reader(*g))
          return false;

        if (match_stem(t.name, "builder.h") && !gen_builder(*g))
          return false;

        if (match_stem(t.name, "reader.h") && !gen_reader(*g))
          return false;

        if (match_stem(t.name, "verifier.h") && !gen_verifier(*g))
          return false;

        if (match_stem(t.name, "json_parser.h") && !gen_json_parser(*g))
          return false;

        if (match_stem(t.name, "json_printer.h") && !gen_json_printer(*g))
          return false;

        t.group = g;
        return true;
      }
    }

    recipe compile_rule::
    apply (action a, target& xt) const
    {
      if (flatcc_c* pt = xt.is_a<flatcc_c> ())
      {
        flatcc_c& t (*pt);

        // Derive file names for the members.
        //
        if (t.h_flatbuffers_common_builder != nullptr)
          t.h_flatbuffers_common_builder->derive_path ();
        if (t.h_flatbuffers_common_reader != nullptr)
          t.h_flatbuffers_common_reader->derive_path ();
        if (t.h_builder != nullptr)
          t.h_builder->derive_path ();
        if (t.h_reader != nullptr)
          t.h_reader->derive_path ();
        if (t.h_verifier != nullptr)
          t.h_verifier->derive_path ();
        if (t.h_json_parser != nullptr)
          t.h_json_parser->derive_path ();
        if (t.h_json_printer != nullptr)
          t.h_json_printer->derive_path ();

        // Inject dependency on the output directory.
        //
        inject_fsdir (a, t);

        // Match prerequisites.
        //
        match_prerequisite_members (a, t);

        // For update inject dependency on the flatcc compiler target.
        //
        if (a == perform_update_id)
          inject (a, t, ctgt);

        switch (a)
        {
        case perform_update_id: return [this] (action a, const target& t)
          {
            return perform_update (a, t);
          };
        case perform_clean_id:  return &perform_clean_group_depdb;
        default:                return noop_recipe; // Configure/dist update.
        }
      }
      else
      {
        const flatcc_c& g (xt.group->as<flatcc_c> ());
        build2::match (a, g);
        return group_recipe; // Execute the group's recipe.
      }
    }

    static void
    append_extension (cstrings& args,
                      const path_target& t,
                      const char* option,
                      const char* default_extension)
    {
      const string* e (t.ext ());
      assert (e != nullptr); // Should have been figured out in apply().

      if (*e != default_extension)
      {
        // flatcc needs the extension with the leading dot (unless it is empty)
        // while we store the extension without. But if there is an extension,
        // then we can get it (with the dot) from the file name.
        //
        args.push_back (option);
        args.push_back (e->empty ()
                        ? e->c_str ()
                        : t.path ().extension_cstring () - 1);
      }
    }

    target_state compile_rule::
    perform_update (action a, const target& xt) const
    {
      tracer trace ("flatcc::compile_rule::perform_update");

      // The rule has been matched which means the members should be resolved
      // and paths assigned. We use the header file as our "target path" for
      // timestamp, depdb, etc.
      //
      const flatcc_c& t (xt.as<flatcc_c> ());

      const process_path& pp (ctgt.process_path ());
      cstrings args {pp.recall_string ()};

      initializer_list<const char*> json_parser_s {"--json", "--json-parser"};
      bool json_parser_o = find_options (json_parser_s, args);

      initializer_list<const char*> json_printer_s {"--json", "--json-printer"};
      bool json_printer_o = find_options (json_printer_s, args);

      initializer_list<const char*> reader_s {"--reader", "-a"};
      bool reader_o = find_options (reader_s, args);

      initializer_list<const char*> builder_s {"--builder", "-a"};
      bool builder_o = find_options (builder_s, args);

      initializer_list<const char*> verifier_s {"--verifier", "-a"};
      bool verifier_o = find_options (verifier_s, args);

      bool binary_schema_o = find_option ("--schema", args);

      bool has_reader = reader_o || !(json_parser_o || json_printer_o || binary_schema_o);

      const c::h* gen_file_for_timestamp = nullptr;
      if (has_reader)
        gen_file_for_timestamp = t.h_reader;
      else if (builder_o)
        gen_file_for_timestamp = t.h_builder;
      else if (verifier_o)
        gen_file_for_timestamp = t.h_verifier;
      else if (json_parser_o)
        gen_file_for_timestamp = t.h_json_parser;
      else if (json_printer_o)
        gen_file_for_timestamp = t.h_json_printer;
      // else if (binary_schema_o)
      //   gen_file_for_timestamp = t.h_binary_schema;

      const path& tp (gen_file_for_timestamp->path ());

      // Update prerequisites and determine if any relevant ones render us
      // out-of-date. Note that currently we treat all the prerequisites as
      // potentially affecting the result (think prologues/epilogues, flatcc
      // compiler target itself, etc).
      //
      timestamp mt (t.load_mtime (tp));
      auto pr (execute_prerequisites<fbs> (a, t, mt));

      bool update (!pr.first);
      target_state ts (update ? target_state::changed : *pr.first);

      const fbs& s (pr.second);

      // Translate paths to relative (to working directory). This results in
      // easier to read diagnostics.

      path relo (relative (t.dir));
      path rels (relative (s.path ()));

      // We use depdb to track changes to the .fbs file name, options,
      // compiler, etc.
      //
      depdb dd (tp + ".d");
      {
        // First should come the rule name/version.
        //
        if (dd.expect ("flatcc.compile 1") != nullptr)
          l4 ([&]{trace << "rule mismatch forcing update of " << t;});

        // Then the compiler checksum.
        //
        if (dd.expect (csum) != nullptr)
          l4 ([&]{trace << "compiler mismatch forcing update of " << t;});

        // Then the options checksum.
        //
        sha256 cs;
        append_options (cs, t, "flatcc.options");

        if (dd.expect (cs.string ()) != nullptr)
          l4 ([&]{trace << "options mismatch forcing update of " << t;});

        // Finally the .fbs input file.
        //
        if (dd.expect (s.path ()) != nullptr)
          l4 ([&]{trace << "input file mismatch forcing update of " << t;});
      }

      // Update if depdb mismatch.
      //
      if (dd.writing () || dd.mtime > mt)
        update = true;

      dd.close ();

      // If nothing changed, then we are done.
      //
      if (!update)
        return ts;

      if (t.h_flatbuffers_common_builder != nullptr)
      {
        args.push_back("--common-builder");
        append_extension (args, *t.h_flatbuffers_common_builder, "--h-suffix", "h");
      }
      if (t.h_flatbuffers_common_reader != nullptr)
      {
        args.push_back("--common-reader");
        append_extension (args, *t.h_flatbuffers_common_reader, "--h-suffix", "h");
      }
      if (t.h_builder != nullptr)
      {
        args.push_back("--builder");
        append_extension (args, *t.h_builder, "--h-suffix", "h");
      }
      if (t.h_reader != nullptr)
      {
        args.push_back("--reader");
        append_extension (args, *t.h_reader, "--h-suffix", "h");
      }
      if (t.h_verifier != nullptr)
      {
        args.push_back("--verifier");
        append_extension (args, *t.h_verifier, "--h-suffix", "h");
      }
      if (t.h_json_parser != nullptr)
      {
        args.push_back("--json_parser");
        append_extension (args, *t.h_json_parser, "--h-suffix", "h");
      }
      if (t.h_json_printer != nullptr)
      {
        args.push_back("--json_printer");
        append_extension (args, *t.h_json_printer, "--h-suffix", "h");
      }

      append_options (args, t, "flatcc.options");

      if (!relo.empty ())
      {
        args.push_back ("-o");
        args.push_back (relo.string ().c_str ());
      }

      args.push_back (rels.string ().c_str ());
      args.push_back (nullptr);

      if (verb >= 2)
        print_process (args);
      else if (verb)
        text << "flatcc " << s;

      if (!t.ctx.dry_run)
      {
        run (pp, args);
        dd.check_mtime (tp);
      }

      t.mtime (system_clock::now ());
      return target_state::changed;
    }
  }
}
