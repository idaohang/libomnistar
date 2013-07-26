remake_add_library(omnistar PREFIX OFF LINK ${LIBELROB_LIBRARIES})
remake_add_headers()
remake_pkg_config_generate(REQUIRES libelrob)
