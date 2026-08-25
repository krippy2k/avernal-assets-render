# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "D:/projects/avernal/avernal-render/../avernal-rhi")
  file(MAKE_DIRECTORY "D:/projects/avernal/avernal-render/../avernal-rhi")
endif()
file(MAKE_DIRECTORY
  "D:/projects/avernal/avernal-render-assets/build/_deps/avernal_rhi-build"
  "D:/projects/avernal/avernal-render-assets/build/_deps/avernal_rhi-subbuild/avernal_rhi-populate-prefix"
  "D:/projects/avernal/avernal-render-assets/build/_deps/avernal_rhi-subbuild/avernal_rhi-populate-prefix/tmp"
  "D:/projects/avernal/avernal-render-assets/build/_deps/avernal_rhi-subbuild/avernal_rhi-populate-prefix/src/avernal_rhi-populate-stamp"
  "D:/projects/avernal/avernal-render-assets/build/_deps/avernal_rhi-subbuild/avernal_rhi-populate-prefix/src"
  "D:/projects/avernal/avernal-render-assets/build/_deps/avernal_rhi-subbuild/avernal_rhi-populate-prefix/src/avernal_rhi-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/projects/avernal/avernal-render-assets/build/_deps/avernal_rhi-subbuild/avernal_rhi-populate-prefix/src/avernal_rhi-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/projects/avernal/avernal-render-assets/build/_deps/avernal_rhi-subbuild/avernal_rhi-populate-prefix/src/avernal_rhi-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
