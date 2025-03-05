cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)
project(lua5.4 VERSION 1.0.0 LANGUAGES C)
set(LUA_COMPAT "-DLUA_COMPAT_5_3")
include("../CMakeLists.txt")