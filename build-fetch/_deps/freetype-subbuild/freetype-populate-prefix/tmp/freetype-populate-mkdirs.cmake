# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/workspaces/RecreatePhoneUi/build-fetch/_deps/freetype-src"
  "/workspaces/RecreatePhoneUi/build-fetch/_deps/freetype-build"
  "/workspaces/RecreatePhoneUi/build-fetch/_deps/freetype-subbuild/freetype-populate-prefix"
  "/workspaces/RecreatePhoneUi/build-fetch/_deps/freetype-subbuild/freetype-populate-prefix/tmp"
  "/workspaces/RecreatePhoneUi/build-fetch/_deps/freetype-subbuild/freetype-populate-prefix/src/freetype-populate-stamp"
  "/workspaces/RecreatePhoneUi/build-fetch/_deps/freetype-subbuild/freetype-populate-prefix/src"
  "/workspaces/RecreatePhoneUi/build-fetch/_deps/freetype-subbuild/freetype-populate-prefix/src/freetype-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/workspaces/RecreatePhoneUi/build-fetch/_deps/freetype-subbuild/freetype-populate-prefix/src/freetype-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/workspaces/RecreatePhoneUi/build-fetch/_deps/freetype-subbuild/freetype-populate-prefix/src/freetype-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
