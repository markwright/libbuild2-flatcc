# libbuild2-hello

Test build system module for `build2`.

There are two ways to set things up: using only the build system, which works
best if you just want to kick the tires, or using the project manager, which
is how you would do it if you were developing a real module and needed the
ability to CI, manage and publish releases, etc.

## Using only the build system

```
git clone .../libbuild2-hello.git
cd libbuild2-hello/
b configure: libbuild2-hello/ config.config.load=~build2
b configure: libbuild2-hello-tests/ config.import.libbuild2_hello=libbuild2-hello/
b test: libbuild2-hello-tests/
```

## Using the project manager

Because the module and the tests have to use different build configurations,
the initialization is more elaborate compared to a typical project:

```
git clone .../libbuild2-hello.git
cd libbuild2-hello/

bdep init --empty

bdep config create --no-default --forward @mod ../libbuild2-hello-build/module/ cc config.config.load=~build2
bdep init @mod -d libbuild2-hello/
b libbuild2-hello/

bdep config create --default --forward @test ../libbuild2-hello-build/tests/ cc config.import.libbuild2_hello=../libbuild2-hello-build/module/
bdep init @test -d libbuild2-hello-tests/ --no-sync
bdep sync
```

Once this is done, we can develop using `bdep` or the build system as usual:

```
bdep test                       # run tests in libbuild2-hello-tests/
b test: libbuild2-hello-tests/  # the same
b libbuild2-hello/              # update the module directly
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
