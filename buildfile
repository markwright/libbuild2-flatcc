# Glue buildfile that "pulls" the tests.
#
# Note that unlike a typical multi-package glue buildfile which pulls all the
# packages, here we only pull the tests package. Pulling the module would be a
# bad idea since then both us and the tests would be building the module
# simultaneously from different build contexts. And that's a big no-no.
#
# One notable consequence of this setup is that the clean operation won't do
# what's expected. We could have tweaked it with an ad hoc recipe that does a
# recursive build system invocation (after cleaning the tests) but such a
# complication does not seem warranted (to do it correctly, we would need to
# merge the resulting target state, etc).
#
import tests = libbuild2-hello-tests/
./: $tests
