#pragma once
#include "global.h"

#define LUA_IDSIZE 60

struct lua_Debug {
  int event;
  const char *name;           // (n)
  const char *namewhat;       // (n) 'global', 'local', 'field', 'method'
  const char *what;           // (S) 'Lua', 'C', 'main', 'tail'
  const char *source;         // (S)
  int currentline;            // (l)
  int nups;                   // (u) number of upvalues
  int linedefined;            // (S)
  char short_src[LUA_IDSIZE]; // (S)
  /* private part */
  int i_ci; // active function
};

struct LuaState;
// lua.org/source/5.0/lstate.h.html#lua_State
typedef struct {
  uint8_t pad[0x44];
  LuaState *LuaState;
} lua_State;

typedef struct luaL_Buffer luaL_Buffer;
typedef const char *(*lua_Chunkreader)(lua_State *L, void *data, size_t *size);
typedef void (*lua_Hook)(lua_State *L, lua_Debug *ar);
typedef void (*userGCFunction)(void *);
typedef int (*lua_CFunction)(lua_State *L);
typedef float lua_Number;

typedef struct luaL_reg {
  const char *name;
  lua_CFunction func;
} luaL_reg;

#define LUA_MULTRET (-1)

#define LUA_REGISTRYINDEX (-10000)
#define LUA_GLOBALSINDEX (-10001)
#define lua_upvalueindex(i) (LUA_GLOBALSINDEX - (i))

#define LUA_OK 0
#define LUA_ERRRUN 1
#define LUA_ERRFILE 2
#define LUA_ERRSYNTAX 3
#define LUA_ERRMEM 4
#define LUA_ERRERR 5

#define LUA_HOOKCALL 0
#define LUA_HOOKRET 1
#define LUA_HOOKLINE 2
#define LUA_HOOKCOUNT 3
#define LUA_HOOKTAILRET 4

#define LUA_MASKCALL (1 << LUA_HOOKCALL)
#define LUA_MASKRET (1 << LUA_HOOKRET)
#define LUA_MASKLINE (1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT (1 << LUA_HOOKCOUNT)

#define LUA_NOREF (-2)
#define LUA_REFNIL (-1)

#define lua_boxpointer(L, u)                                                   \
  (*(void **)(lua_newuserdata(L, sizeof(void *))) = (u))
#define lua_unboxpointer(L, i) (*(void **)(lua_touserdata(L, i)))

#define lua_pop(L, n) lua_settop(L, -(n) - 1)

#define lua_register(L, n, f)                                                  \
  (lua_pushstring(L, n), lua_pushcfunction(L, f),                              \
   lua_settable(L, LUA_GLOBALSINDEX))

#define lua_pushcfunction(L, f) lua_pushcclosure(L, f, 0)

#define lua_isnoneornil(L, n) (lua_type(L, n) <= 0)
#define lua_isnone(L, n) (lua_type(L, n) == LUA_TNONE)
#define lua_isnil(L, n) (lua_type(L, n) == LUA_TNIL)
#define lua_isboolean(L, n) (lua_type(L, n) == LUA_TBOOLEAN)
#define lua_islightuserdata(L, n) (lua_type(L, n) == LUA_TLIGHTUSERDATA)
#define lua_istable(L, n) (lua_type(L, n) == LUA_TTABLE)
#define lua_iscfunction(L, n) (lua_type(L, n) == LUA_CFUNCTION)
#define lua_isfunction(L, n) (lua_type(L, n) == LUA_TFUNCTION)
#define lua_isuserdata(L, n) (lua_type(L, n) == LUA_TUSERDATA)
#define lua_isthread(L, n) (lua_type(L, n) == LUA_TTHREAD)
#define lua_isproto(L, n) (lua_type(L, n) == LUA_TPROTO)
#define lua_isupvalue(L, n) (lua_type(L, n) == LUA_TUPVALUE)

#define lua_pushliteral(L, s)                                                  \
  lua_pushlstring(L, "" s, (sizeof(s) / sizeof(char)) - 1)

#define lua_getregistry(L) lua_pushvalue(L, LUA_REGISTRYINDEX)
#define lua_setglobal(L, s)                                                    \
  (lua_pushstring(L, s), lua_insert(L, -2), lua_settable(L, LUA_GLOBALSINDEX))
#define lua_getglobal(L, s)                                                    \
  (lua_pushstring(L, s), lua_gettable(L, LUA_GLOBALSINDEX))

#define lua_ref(L, lock)                                                       \
  ((lock) ? luaL_ref(L, LUA_REGISTRYINDEX)                                     \
          : (lua_pushstring(L, "unlocked references are obsolete"),            \
             lua_error(L), 0))
#define lua_unref(L, ref) luaL_unref(L, LUA_REGISTRYINDEX, (ref))
#define lua_getref(L, ref) lua_rawgeti(L, LUA_REGISTRYINDEX, ref)
#define abs_index(L, i)                                                        \
  ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)

union Value {
  void *p;
  float n;
  int b;
};
// lua.org/source/5.0/lobject.h.html#TObject
struct TObject {
  int tt;
  Value value;
};

VALIDATE_SIZE(TObject, 8);
struct RRef {
  void *d;
  void *t;
};

struct LuaState;
struct LuaStackObject {
  LuaState *m_state;
  int m_stackIndex;
};
VALIDATE_SIZE(LuaStackObject, 8);

struct LuaObject { // 0x14 bytes
  LuaObject *m_next;
  LuaObject *m_prev;
  LuaState *m_state;
  TObject m_object;
};
VALIDATE_SIZE(LuaObject, 0x14);

enum StandardLibraries { LIB_NONE, LIB_BASE, LIB_OSIO };
struct LuaState { // 0x34 bytes
  lua_State *m_state;
  void *ForMultipleThreads;
  bool m_ownState;
  LuaObject m_threadObj;
  LuaState *m_rootState;
  struct MiniLuaObject {
    LuaObject *m_next; // only valid when in free list
    LuaObject *m_prev; // only valid when in used list
  } m_headObject, m_tailObject;
};
VALIDATE_SIZE(LuaState, 0x34);
