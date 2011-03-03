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
  HOME "http://aslforge.ethz.ch/projects/libomnistar"
  LICENSE "GNU Lesser General Public License (LGPL)"
  PREFIX omnistar-
  INSTALL /usr
)

remake_doc(
  html
)

remake_pack_deb(
  libomnistar libelrob
)
remake_pack_deb(
  COMPONENT dev
  DESCRIPTION "development headers"
  libomnistar libelrob-dev
)
remake_pack_deb(
  ARCH all
  COMPONENT doc
  DESCRIPTION "documentation"
)
