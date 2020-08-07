# libbuild2-flatcc

Test build system module for `build2`.

There are two ways to set things up: using only the build system, which works
best if you just want to kick the tires, or using the project manager, which
is how you would do it if you were developing a real module and needed the
ability to CI, manage and publish releases, etc.

## Using only the build system

```
git clone .../libbuild2-flatcc.git
cd libbuild2-flatcc/
b configure: libbuild2-flatcc/ config.config.load=~build2
b configure: libbuild2-flatcc-tests/ config.import.libbuild2_flatcc=libbuild2-flatcc/
b test
```

## Using the project manager

Because the module and the tests have to use different build configurations,
the initialization is more elaborate compared to a typical project:

```
git clone .../libbuild2-flatcc.git
cd libbuild2-flatcc/

bdep init --empty

bdep config create --no-default --forward @mod ../libbuild2-flatcc-build/module/ cc config.config.load=~build2
bdep init @mod -d libbuild2-flatcc/
b libbuild2-flatcc/

bdep config create --default --forward @test ../libbuild2-flatcc-build/tests/ cc config.import.libbuild2_flatcc=../libbuild2-flatcc-build/module/
bdep init @test -d libbuild2-flatcc-tests/ --no-sync
bdep sync
```

Once this is done, we can develop using `bdep` or the build system as usual:

```
bdep test                       # run tests in libbuild2-flatcc-tests/
b test                          # the same
b libbuild2-flatcc/             # update the module directly
```

We can also CI our module, manage releases, and publish it to the package
repository:

```
bdep ci        # submits only the module (which pulls in the tests)

bdep release   # releases both the module and the tests

bdep publish   # submits both the module and the tests
```

Note that the `bdep-ci` and `bdep-publish` commands are tweaked to do the
right thing in this setup with the default options files located in the
`.build2/` subdirectory.
