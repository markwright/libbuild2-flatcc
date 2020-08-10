#include "basics.hxx"

int create_T(flatcc_builder_t *B) {
  // Basics_T_create(
}

int main (int argc, char *argv[]) {
  flatcc_builder_t builder;
  (void)argc;
  (void)argv;
  flatcc_builder_init(&builder);
  int rc = create_T(&builder);
}
