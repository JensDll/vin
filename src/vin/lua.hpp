#pragma once

extern "C" {
#include <lauxlib.h> // IWYU pragma: export
#include <lua.h> // IWYU pragma: export
#include <lualib.h> // IWYU pragma: export
}

#define VIN_LUA_EXPECT_NARGS(n)                                                                        \
  do {                                                                                                 \
    if (lua_gettop(L) != (n)) {                                                                        \
      return luaL_error(L, "invalid number of arguments - expected " #n " but got %d", lua_gettop(L)); \
    }                                                                                                  \
  } while (false)

#define VIN_LUA_EXPECT_TYPE(t, n)                                                                        \
  do {                                                                                                   \
    if (!lua_is##t(L, (n))) {                                                                            \
      if ((n) == 1) {                                                                                    \
        return luaL_error(                                                                               \
          L, "invalid first argument - expected " #t " but got %s", lua_typename(L, lua_type(L, (n))));  \
      }                                                                                                  \
      if ((n) == 2) {                                                                                    \
        return luaL_error(                                                                               \
          L, "invalid second argument - expected " #t " but got %s", lua_typename(L, lua_type(L, (n)))); \
      }                                                                                                  \
      if ((n) == 3) {                                                                                    \
        return luaL_error(                                                                               \
          L, "invalid third argument - expected " #t " but got %s", lua_typename(L, lua_type(L, (n))));  \
      }                                                                                                  \
      return luaL_error(                                                                                 \
        L, "invalid " #n "th argument - expected " #t " but got %s", lua_typename(L, lua_type(L, (n)))); \
    }                                                                                                    \
  } while (false)

#define VIN_LUA_EXPECT_TABLE(n) VIN_LUA_EXPECT_TYPE(table, n)

#define VIN_LUA_ASSERT_POP(L, n)    \
  do {                              \
    g_assert(lua_gettop(L) == (n)); \
    lua_pop(L, (n));                \
  } while (false)
