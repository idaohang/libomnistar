cmake_minimum_required(VERSION 2.6)

include(ReMake)

remake_project(
  libomnistar
  VERSION 0.1
  RELEASE alpha
  SUMMARY "Driver library for the OmniSTAR GPS guidance system."
  AUTHOR "Pierre Lamon"
  AUTHOR "Ralf Kaestner"
  CONTACT "ralf.kaestner@gmail.com"
  HOME "http://github.com/kralf/libomnistar"
  LICENSE "GNU Lesser General Public License (LGPL)"
  PREFIX omnistar-
  INSTALL /usr
)

remake_doc(
  html
)

remake_pack_deb(
  DEPENDS libelrob
)
remake_pack_deb(
  COMPONENT dev
  DESCRIPTION "development headers"
  DEPENDS libomnistar libelrob-dev
)
remake_pack_deb(
  ARCH all
  COMPONENT doc
  DESCRIPTION "documentation"
)

remake_distribute_deb(
  DISTRIBUTION lucid
  SECTION libs
  UPLOAD ppa:kralf/asl
  DEPENDS libelrob-dev remake pkg-config doxygen
  PASS CMAKE_BUILD_TYPE LIBOMNISTAR_GIT_REVISION
)
remake_distribute_deb(
  DISTRIBUTION precise
  SECTION libs
  UPLOAD ppa:kralf/asl
  DEPENDS libelrob-dev remake pkg-config doxygen
  PASS CMAKE_BUILD_TYPE LIBOMNISTAR_GIT_REVISION
)
remake_distribute_deb(
  DISTRIBUTION trusty
  SECTION libs
  UPLOAD ppa:kralf/asl
  DEPENDS libelrob-dev remake pkg-config doxygen
  PASS CMAKE_BUILD_TYPE LIBOMNISTAR_GIT_REVISION
)
