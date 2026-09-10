# CMake generated Testfile for 
# Source directory: C:/Users/kirpa/Study/Chess-game
# Build directory: C:/Users/kirpa/Study/Chess-game/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(perft "C:/Users/kirpa/Study/Chess-game/build/Debug/perft_test.exe")
  set_tests_properties(perft PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kirpa/Study/Chess-game/CMakeLists.txt;23;add_test;C:/Users/kirpa/Study/Chess-game/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(perft "C:/Users/kirpa/Study/Chess-game/build/Release/perft_test.exe")
  set_tests_properties(perft PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kirpa/Study/Chess-game/CMakeLists.txt;23;add_test;C:/Users/kirpa/Study/Chess-game/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(perft "C:/Users/kirpa/Study/Chess-game/build/MinSizeRel/perft_test.exe")
  set_tests_properties(perft PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kirpa/Study/Chess-game/CMakeLists.txt;23;add_test;C:/Users/kirpa/Study/Chess-game/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(perft "C:/Users/kirpa/Study/Chess-game/build/RelWithDebInfo/perft_test.exe")
  set_tests_properties(perft PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/kirpa/Study/Chess-game/CMakeLists.txt;23;add_test;C:/Users/kirpa/Study/Chess-game/CMakeLists.txt;0;")
else()
  add_test(perft NOT_AVAILABLE)
endif()
