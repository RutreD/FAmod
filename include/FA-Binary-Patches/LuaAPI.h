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
struct lua_State;

typedef struct luaL_Buffer {
  char *p;   // current position in buffer
  int lvl;   // number of strings in the stack (level)
  lua_State *L;
  char buffer[BUFSIZ];
} luaL_Buffer;

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

enum LuaType {
  LUA_TNONE          = -1,
  LUA_TNIL           = 0,
  LUA_TBOOLEAN       = 1,
  LUA_TLIGHTUSERDATA = 2,
  LUA_TNUMBER        = 3,
  LUA_TSTRING        = 4,
  LUA_TTABLE         = 5,
  LUA_TFUNCTION      = 6,
  LUA_TUSERDATA      = 7,
  LUA_TTHREAD        = 8,
  LUA_TWSTRING       = 9
};

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

struct RType;

struct LuaStackObject {
  LuaState *m_state;
  int m_stackIndex;
};
VALIDATE_SIZE(LuaStackObject, 8);

inline const char *const *luaT_typenames = reinterpret_cast<const char *const *>(0x00D474D8);

enum StandardLibraries { LIB_NONE, LIB_BASE, LIB_OSIO };

struct LuaObject;

// --------------------------------------------------------------------------
// LuaState (LuaPlus)
// --------------------------------------------------------------------------
struct LuaState { // 0x34 bytes
  lua_State *m_state;
  void *ForMultipleThreads;
  bool m_ownState;
  uint8_t m_threadObj[0x14]; // LuaObject (size 0x14)
  LuaState *m_rootState;
  struct MiniLuaObject {
    LuaObject *m_next; // only valid when in free list
    LuaObject *m_prev; // only valid when in used list
  } m_headObject, m_tailObject;

  LuaState() = default;
  ~LuaState() {
    reinterpret_cast<void(__thiscall *)(LuaState*)>(0x0090A600)(this);
  }

  LuaState(StandardLibraries libs) {
    reinterpret_cast<void(__thiscall *)(LuaState*, StandardLibraries)>(0x0090AC10)(this, libs);
  }

  LuaState(LuaState *parentState) {
    reinterpret_cast<void(__thiscall *)(LuaState*, LuaState*)>(0x0090A520)(this, parentState);
  }

  LuaState(int unused) {
    reinterpret_cast<void(__thiscall *)(LuaState*, int)>(0x0090A5D0)(this, unused);
  }

  inline LuaObject GetGlobal(const char *key);
  inline void GetGlobals(LuaObject *out);

  LuaState *GetActiveState() {
    return reinterpret_cast<LuaState*(__thiscall *)(LuaState*)>(0x0090BEE0)(this);
  }

  lua_State *GetActiveCState() {
    return reinterpret_cast<lua_State*(__thiscall *)(LuaState*)>(0x0090BEF0)(this);
  }

  const char *CheckString(int narg, size_t *len = nullptr) {
    return reinterpret_cast<const char*(__thiscall *)(LuaState*, int, size_t*)>(0x0090BFB0)(this, narg, len);
  }

  int ArgError(int narg, const char *msg) {
    return reinterpret_cast<int(__thiscall *)(LuaState*, int, const char*)>(0x0090BF70)(this, narg, msg);
  }

  void CheckAny(int narg) {
    reinterpret_cast<void(__thiscall *)(LuaState*, int)>(0x0090C170)(this, narg);
  }

  template <typename... Args>
  int Error(const char *fmt, Args... args) {
    return reinterpret_cast<int(__cdecl *)(const char*, ...)>(0x0090C1D0)(fmt, args...);
  }

  static LuaState *GetUserLuaState() {
    return reinterpret_cast<LuaState*(__cdecl *)()>(0x008C65B0)();
  }
};
VALIDATE_SIZE(LuaState, 0x34);

// --------------------------------------------------------------------------
// LuaObject (LuaPlus)
// --------------------------------------------------------------------------
struct LuaObject { // 0x14 bytes
  LuaObject *m_next{nullptr};
  LuaObject *m_prev{nullptr};
  LuaState *m_state{nullptr};
  TObject m_object{0, {0}};

  LuaObject() {
    reinterpret_cast<void(__thiscall *)(LuaObject*)>(0x00908920)(this);
  }

  LuaObject(LuaState *state) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*)>(0x00908970)(this, state);
  }

  LuaObject(LuaState *state, int stackIndex) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*, int)>(0x009089D0)(this, state, stackIndex);
  }

  LuaObject(const LuaObject &other) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const LuaObject*)>(0x00908A40)(this, &other);
  }

  LuaObject(const LuaStackObject &stack) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const LuaStackObject*)>(0x00908AC0)(this, &stack);
  }

  LuaObject(LuaState *state, const TObject *obj) {
    this->m_next = nullptr;
    this->m_prev = nullptr;
    this->m_state = nullptr;
    this->m_object.tt = 0;
    if (state && obj) {
      reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*, const TObject*)>(0x009088E0)(this, state, obj);
    }
  }

  ~LuaObject() {
    reinterpret_cast<void(__thiscall *)(LuaObject*)>(0x009075D0)(this);
  }

  LuaObject &operator=(const LuaObject &other) {
    if (this != &other) {
      reinterpret_cast<void(__thiscall *)(LuaObject*)>(0x009075D0)(this);
      if (other.m_state) {
        reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*, const TObject*)>(0x009088E0)(this, other.m_state, &other.m_object);
      }
    }
    return *this;
  }

  LuaObject &operator=(const LuaStackObject &stack) {
    reinterpret_cast<void(__thiscall *)(LuaObject*)>(0x009075D0)(this);
    reinterpret_cast<void(__thiscall *)(LuaObject*, const LuaStackObject*)>(0x00908AC0)(this, &stack);
    return *this;
  }

  LuaObject operator[](const char *key) const {
    LuaObject out;
    reinterpret_cast<void(__thiscall *)(const LuaObject*, LuaObject*, const char*)>(0x00908F60)(this, &out, key);
    return out;
  }

  LuaObject operator[](int key) const {
    LuaObject out;
    reinterpret_cast<void(__thiscall *)(const LuaObject*, LuaObject*, int)>(0x009091E0)(this, &out, key);
    return out;
  }

  bool operator==(const LuaObject &other) const {
    return reinterpret_cast<int(__cdecl *)(lua_State*, const TObject*, const TObject*)>(0x00929810)(
               GetActiveCState(), &this->m_object, &other.m_object) != 0;
  }

  bool operator!=(const LuaObject &other) const {
    return !(*this == other);
  }

  operator bool() const {
    return GetBoolean();
  }

  int Type() const {
    return this->m_object.tt;
  }

  bool IsNil() const {
    return reinterpret_cast<bool(__thiscall *)(const LuaObject*)>(0x009072F0)(this);
  }

  bool IsTable() const {
    return reinterpret_cast<bool(__thiscall *)(const LuaObject*)>(0x00907310)(this);
  }

  bool IsUserData() const {
    return reinterpret_cast<bool(__thiscall *)(const LuaObject*)>(0x00907330)(this);
  }

  bool IsString() const {
    return reinterpret_cast<bool(__thiscall *)(const LuaObject*)>(0x00907350)(this);
  }

  bool IsNumber() const {
    return reinterpret_cast<bool(__thiscall *)(const LuaObject*)>(0x00907370)(this);
  }

  bool IsBoolean() const {
    return reinterpret_cast<bool(__thiscall *)(const LuaObject*)>(0x00907390)(this);
  }

  bool IsFunction() const {
    return reinterpret_cast<bool(__thiscall *)(const LuaObject*)>(0x00907810)(this);
  }

  bool IsConvertibleToString() const {
    return reinterpret_cast<bool(__thiscall *)(const LuaObject*)>(0x009077C0)(this);
  }

  bool GetBoolean() const {
    return reinterpret_cast<bool(__thiscall *)(const LuaObject*)>(0x00907C90)(this);
  }

  int GetInteger() const {
    return reinterpret_cast<int(__thiscall *)(const LuaObject*)>(0x00907910)(this);
  }

  lua_Number GetNumber() const {
    return reinterpret_cast<lua_Number(__thiscall *)(const LuaObject*)>(0x00907970)(this);
  }

  lua_Number GetDouble() const {
    return reinterpret_cast<lua_Number(__thiscall *)(const LuaObject*)>(0x00907A30)(this);
  }

  const char *GetString() const {
    return reinterpret_cast<const char*(__thiscall *)(const LuaObject*)>(0x00907A90)(this);
  }

  const char *ToString() const {
    return reinterpret_cast<const char*(__thiscall *)(const LuaObject*)>(0x009073E0)(this);
  }

  lua_Number ToNumber() const {
    return reinterpret_cast<lua_Number(__thiscall *)(const LuaObject*)>(0x009073B0)(this);
  }

  const char *TypeName() const {
    int t = Type();
    if (t == LUA_TNONE) return "no value";
    return luaT_typenames[t];
  }

  void GetUserData(RRef *out) const {
    reinterpret_cast<void(__thiscall *)(const LuaObject*, RRef*)>(0x00907BC0)(this, out);
  }

  int GetCount() const {
    return reinterpret_cast<int(__thiscall *)(const LuaObject*)>(0x00907F50)(this);
  }

  int GetN() const {
    return reinterpret_cast<int(__thiscall *)(const LuaObject*)>(0x00907E50)(this);
  }

  int GetTableCount() const {
    return reinterpret_cast<int(__thiscall *)(const LuaObject*)>(0x0090A410)(this);
  }

  LuaState *GetActiveState() const {
    return reinterpret_cast<LuaState*(__thiscall *)(const LuaObject*)>(0x009072B0)(this);
  }

  lua_State *GetActiveCState() const {
    return reinterpret_cast<lua_State*(__thiscall *)(const LuaObject*)>(0x009072C0)(this);
  }

  LuaObject GetMetaTable() const {
    LuaObject out;
    reinterpret_cast<void(__thiscall *)(const LuaObject*, LuaObject*)>(0x00908BA0)(this, &out);
    return out;
  }

  LuaObject Clone() const {
    LuaObject out;
    reinterpret_cast<void(__thiscall *)(const LuaObject*, LuaObject*)>(0x0090A180)(this, &out);
    return out;
  }

  void CreateTable(LuaObject *out, const char *key, int narray = 0, int lnhash = 0) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaObject*, const char*, int, int)>(0x00908C10)(this, out, key, narray, lnhash);
  }

  void CreateTable(LuaObject *out, int key, int narray = 0, int lnhash = 0) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaObject*, int, int, int)>(0x00908CA0)(this, out, key, narray, lnhash);
  }

  void GetByName(LuaObject *out, const char *name) const {
    reinterpret_cast<void(__thiscall *)(const LuaObject*, LuaObject*, const char*)>(0x0090A160)(this, out, name);
  }

  void Lookup(LuaObject *out, const char *key) const {
    reinterpret_cast<void(__thiscall *)(const LuaObject*, LuaObject*, const char*)>(0x009093B0)(this, out, key);
  }

  void PushStack(LuaStackObject *out, LuaState *state) const {
    reinterpret_cast<void(__thiscall *)(const LuaObject*, LuaStackObject*, LuaState*)>(0x00907D80)(this, out, state);
  }

  void PushStack(lua_State *L) const {
    reinterpret_cast<void(__thiscall *)(const LuaObject*, lua_State*)>(0x00907D10)(this, L);
  }

  void AssignNil(LuaState *state) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*)>(0x009095C0)(this, state);
  }

  void AssignBoolean(LuaState *state, bool value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*, bool)>(0x00909600)(this, state, value);
  }

  void AssignInteger(LuaState *state, int value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*, int)>(0x00909650)(this, state, value);
  }

  void AssignNumber(LuaState *state, float value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*, float)>(0x009096A0)(this, state, value);
  }

  void AssignThread(LuaState *state) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*)>(0x009096F0)(this, state);
  }

  void AssignString(LuaState *state, const char *value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*, const char*)>(0x00909750)(this, state, value);
  }

  void AssignNewTable(LuaState *state, int narray = 0, int lnhash = 0) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*, int, int)>(0x00909940)(this, state, narray, lnhash);
  }

  void AssignTObject(LuaState *state, const TObject *value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaState*, const TObject*)>(0x009099B0)(this, state, value);
  }

  void Insert(LuaObject *value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, LuaObject*)>(0x00909AF0)(this, value);
  }

  void Insert(int key, LuaObject *value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, int, LuaObject*)>(0x00909CE0)(this, key, value);
  }

  void Register(const char *name, lua_CFunction func, int nupvalues = 0) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const char*, lua_CFunction, int)>(0x00907630)(this, name, func, nupvalues);
  }

  void Reset() {
    reinterpret_cast<void(__thiscall *)(LuaObject*)>(0x00907600)(this);
  }

  void SetBoolean(const char *key, bool value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const char*, bool)>(0x009080C0)(this, key, value);
  }

  void SetInteger(const char *key, int value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const char*, int)>(0x009081F0)(this, key, value);
  }

  void SetInteger(int key, int value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, int, int)>(0x00908240)(this, key, value);
  }

  void SetNumber(const char *key, float value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const char*, float)>(0x00908320)(this, key, value);
  }

  void SetNumber(int key, float value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, int, float)>(0x00908370)(this, key, value);
  }

  void SetString(const char *key, const char *value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const char*, const char*)>(0x00908450)(this, key, value);
  }

  void SetString(int key, const char *value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, int, const char*)>(0x009084E0)(this, key, value);
  }

  void SetObject(const char *key, const LuaObject &value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const char*, const LuaObject*)>(0x00908760)(this, key, &value);
  }

  void SetObject(int key, const LuaObject &value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, int, const LuaObject*)>(0x009087A0)(this, key, &value);
  }

  void SetObject(const LuaObject &key, const LuaObject &value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const LuaObject*, const LuaObject*)>(0x00908810)(this, &key, &value);
  }

  void SetNil(const char *key) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const char*)>(0x00907FA0)(this, key);
  }

  void SetNil(int key) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, int)>(0x00907FF0)(this, key);
  }

  void SetMetaTable(const LuaObject &value) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const LuaObject*)>(0x00907E00)(this, &value);
  }

  void SetN(int n) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, int)>(0x00907ED0)(this, n);
  }

  void TypeError(const char *msg) {
    reinterpret_cast<void(__thiscall *)(LuaObject*, const char*)>(0x009072D0)(this, msg);
  }
};
VALIDATE_SIZE(LuaObject, 0x14);

inline LuaObject LuaState::GetGlobal(const char *key) {
  LuaObject out;
  reinterpret_cast<void(__thiscall *)(LuaState*, LuaObject*, const char*)>(0x004579D0)(this, &out, key);
  return out;
}

inline void LuaState::GetGlobals(LuaObject *out) {
  reinterpret_cast<void(__thiscall *)(LuaState*, LuaObject*)>(0x0090A690)(this, out);
}

// --------------------------------------------------------------------------
// Lua C API function pointers
// --------------------------------------------------------------------------
inline const auto lua_gettop             = reinterpret_cast<int(__cdecl *)(lua_State *L)>(0x0090C590);
inline const auto lua_settop             = reinterpret_cast<void(__cdecl *)(lua_State *L, int index)>(0x0090C5A0);
inline const auto lua_pushvalue          = reinterpret_cast<void(__cdecl *)(lua_State *L, int index)>(0x0090C6E0);
inline const auto lua_remove             = reinterpret_cast<void(__cdecl *)(lua_State *L, int index)>(0x0090C5F0);
inline const auto lua_insert             = reinterpret_cast<void(__cdecl *)(lua_State *L, int index)>(0x0090C640);
inline const auto lua_replace            = reinterpret_cast<void(__cdecl *)(lua_State *L, int index)>(0x0090C690);
inline const auto lua_checkstack         = reinterpret_cast<int(__cdecl *)(lua_State *L, int size)>(0x0090C460);
inline const auto lua_xmove              = reinterpret_cast<void(__cdecl *)(lua_State *from, lua_State *to, int n)>(0x0090C4C0);
inline const auto lua_isnumber           = reinterpret_cast<int(__cdecl *)(lua_State *L, int index)>(0x0090C7A0);
inline const auto lua_isstring           = reinterpret_cast<int(__cdecl *)(lua_State *L, int index)>(0x0090C800);
inline const auto lua_type               = reinterpret_cast<int(__cdecl *)(lua_State *L, int index)>(0x0090C740);
inline const auto lua_typename           = reinterpret_cast<const char*(__cdecl *)(lua_State *L, int t)>(0x0090C780);
inline const auto lua_rawequal           = reinterpret_cast<int(__cdecl *)(lua_State *L, int index1, int index2)>(0x0090C890);
inline const auto lua_lessthan           = reinterpret_cast<int(__cdecl *)(lua_State *L, int index1, int index2)>(0x0090C980);
inline const auto lua_tonumber           = reinterpret_cast<lua_Number(__cdecl *)(lua_State *L, int index)>(0x0090C9F0);
inline const auto lua_toboolean          = reinterpret_cast<int(__cdecl *)(lua_State *L, int index)>(0x0090CA40);
inline const auto lua_tostring           = reinterpret_cast<const char*(__cdecl *)(lua_State *L, int index)>(0x0090CA90);
inline const auto lua_strlen             = reinterpret_cast<size_t(__cdecl *)(lua_State *L, int index)>(0x0090CB10);
inline const auto lua_touserdata         = reinterpret_cast<void*(__cdecl *)(lua_State *L, int index)>(0x0090CBB0);
inline const auto lua_tolightuserdata    = reinterpret_cast<void*(__cdecl *)(lua_State *L, int index)>(0x0090CC10);
inline const auto lua_tothread           = reinterpret_cast<lua_State*(__cdecl *)(lua_State *L, int index)>(0x0090CC50);
inline const auto lua_topointer          = reinterpret_cast<const void*(__cdecl *)(lua_State *L, int index)>(0x0090CC90);

inline const auto lua_pushnil            = reinterpret_cast<void(__cdecl *)(lua_State *L)>(0x0090CD00);
inline const auto lua_pushnumber         = reinterpret_cast<void(__cdecl *)(lua_State *L, lua_Number n)>(0x0090CD40);
inline const auto lua_pushlstring        = reinterpret_cast<void(__cdecl *)(lua_State *L, const char *s, size_t len)>(0x0090CD80);
inline const auto lua_pushstring         = reinterpret_cast<void(__cdecl *)(lua_State *L, const char *s)>(0x0090CDF0);
inline const auto lua_pushvfstring       = reinterpret_cast<const char*(__cdecl *)(lua_State *L, const char *fmt, va_list argp)>(0x0090CE50);
inline const auto lua_pushfstring        = reinterpret_cast<const char*(__cdecl *)(lua_State *L, const char *fmt, ...)>(0x0090CE90);
inline const auto lua_pushcclosure       = reinterpret_cast<void(__cdecl *)(lua_State *L, lua_CFunction fn, int n)>(0x0090CED0);
inline const auto lua_pushboolean        = reinterpret_cast<void(__cdecl *)(lua_State *L, int b)>(0x0090CF80);
inline const auto lua_pushlightuserdata  = reinterpret_cast<void(__cdecl *)(lua_State *L, void *p)>(0x0090CFC0);

inline const auto lua_gettable           = reinterpret_cast<void(__cdecl *)(lua_State *L, int index)>(0x0090D000);
inline const auto lua_rawget             = reinterpret_cast<void(__cdecl *)(lua_State *L, int index)>(0x0090D050);
inline const auto lua_rawgeti            = reinterpret_cast<void(__cdecl *)(lua_State *L, int index, int n)>(0x0090D0A0);
inline const auto lua_newtable           = reinterpret_cast<void(__cdecl *)(lua_State *L)>(0x0090D110);
inline const auto lua_getmetatable       = reinterpret_cast<int(__cdecl *)(lua_State *L, int objindex)>(0x0090D180);
inline const auto lua_getfenv            = reinterpret_cast<void(__cdecl *)(lua_State *L, int index)>(0x0090D1F0);

inline const auto lua_settable           = reinterpret_cast<void(__cdecl *)(lua_State *L, int index)>(0x0090D260);
inline const auto lua_rawset             = reinterpret_cast<void(__cdecl *)(lua_State *L, int index)>(0x0090D2A0);
inline const auto lua_rawseti            = reinterpret_cast<void(__cdecl *)(lua_State *L, int index, int n)>(0x0090D2F0);
inline const auto lua_setmetatable       = reinterpret_cast<int(__cdecl *)(lua_State *L, int objindex)>(0x0090D340);
inline const auto lua_setfenv            = reinterpret_cast<int(__cdecl *)(lua_State *L, int index)>(0x0090D3B0);

inline const auto lua_call               = reinterpret_cast<void(__cdecl *)(lua_State *L, int nargs, int nresults)>(0x0090D400);
inline const auto lua_pcall              = reinterpret_cast<int(__cdecl *)(lua_State *L, int nargs, int nresults, int errfunc)>(0x0090D430);
inline const auto lua_load               = reinterpret_cast<int(__cdecl *)(lua_State *L, lua_Chunkreader reader, void *dt, const char *chunkname)>(0x0090D5C0);

inline const auto lua_getgcthreshold     = reinterpret_cast<int(__cdecl *)(lua_State *L)>(0x0090D650);
inline const auto lua_getgccount         = reinterpret_cast<int(__cdecl *)(lua_State *L)>(0x0090D660);
inline const auto lua_setgcthreshold     = reinterpret_cast<void(__cdecl *)(lua_State *L, int newthreshold)>(0x0090D670);

inline const auto lua_error              = reinterpret_cast<int(__cdecl *)(lua_State *L)>(0x0090D6C0);
inline const auto lua_next               = reinterpret_cast<int(__cdecl *)(lua_State *L, int index)>(0x0090D6D0);
inline const auto lua_concat             = reinterpret_cast<void(__cdecl *)(lua_State *L, int n)>(0x0090D740);

// Auxiliary Library (luaL)
inline const auto luaL_openlib           = reinterpret_cast<void(__cdecl *)(lua_State *L, const char *libname, const luaL_reg *l, int nup)>(0x0090DE00);
inline const auto luaL_getmetafield      = reinterpret_cast<int(__cdecl *)(lua_State *L, int obj, const char *e)>(0x0090DD40);
inline const auto luaL_callmeta          = reinterpret_cast<int(__cdecl *)(lua_State *L, int obj, const char *e)>(0x0090DDA0);
inline const auto luaL_typerror          = reinterpret_cast<int(__cdecl *)(lua_State *L, int narg, const char *tname)>(0x0090E9A0);
inline const auto luaL_argerror          = reinterpret_cast<int(__cdecl *)(lua_State *L, int numarg, const char *extramsg)>(0x0090E900);
inline const auto luaL_checklstring      = reinterpret_cast<const char*(__cdecl *)(lua_State *L, int numarg, size_t *l)>(0x0090EAA0);
inline const auto luaL_optlstring        = reinterpret_cast<const char*(__cdecl *)(lua_State *L, int numarg, const char *def, size_t *l)>(0x0090EB10);
inline const auto luaL_checknumber       = reinterpret_cast<lua_Number(__cdecl *)(lua_State *L, int numarg)>(0x0090EB70);
inline const auto luaL_optnumber         = reinterpret_cast<lua_Number(__cdecl *)(lua_State *L, int narg, lua_Number def)>(0x0090EBF0);
inline const auto luaL_checkany          = reinterpret_cast<void(__cdecl *)(lua_State *L, int narg)>(0x0090EA70);
inline const auto luaL_checktype         = reinterpret_cast<void(__cdecl *)(lua_State *L, int narg, int t)>(0x0090EA20);
inline const auto luaL_newmetatable      = reinterpret_cast<int(__cdecl *)(lua_State *L, const char *tname)>(0x0090DC70);
inline const auto luaL_getmetatable      = reinterpret_cast<void(__cdecl *)(lua_State *L, const char *tname)>(0x0090DCF0);
inline const auto luaL_where             = reinterpret_cast<void(__cdecl *)(lua_State *L, int lvl)>(0x0090DB80);
inline const auto luaL_findstring        = reinterpret_cast<int(__cdecl *)(const char *name, const char *const list[])>(0x0090DC20);
inline const auto luaL_buffinit          = reinterpret_cast<void(__cdecl *)(lua_State *L, luaL_Buffer *B)>(0x0090E400);
inline const auto luaL_prepbuffer        = reinterpret_cast<char*(__cdecl *)(luaL_Buffer *B)>(0x0090E260);
inline const auto luaL_addlstring        = reinterpret_cast<void(__cdecl *)(luaL_Buffer *B, const char *s, size_t l)>(0x0090E2A0);
inline const auto luaL_addstring         = reinterpret_cast<void(__cdecl *)(luaL_Buffer *B, const char *s)>(0x0090E300);
inline const auto luaL_addvalue          = reinterpret_cast<void(__cdecl *)(luaL_Buffer *B)>(0x0090E370);
inline const auto luaL_pushresult        = reinterpret_cast<void(__cdecl *)(luaL_Buffer *B)>(0x0090E330);
inline const auto luaL_loadfile          = reinterpret_cast<int(__cdecl *)(lua_State *L, const char *filename)>(0x0090E5D0);
inline const auto luaL_loadbuffer        = reinterpret_cast<int(__cdecl *)(lua_State *L, const char *buff, size_t sz, const char *name)>(0x0090E760);

// Convenient helper functions
inline int lua_dostring(lua_State *L, const char *str) {
  return luaL_loadbuffer(L, str, strlen(str), str) || lua_pcall(L, 0, LUA_MULTRET, 0);
}

inline int lua_dobuffer(lua_State *L, const char *buff, size_t sz, const char *n) {
  return luaL_loadbuffer(L, buff, sz, n) || lua_pcall(L, 0, LUA_MULTRET, 0);
}

#define lua_pop(L, n) lua_settop(L, -(n) - 1)
#define lua_register(L, n, f) \
  (lua_pushstring(L, n), lua_pushcfunction(L, f), lua_settable(L, LUA_GLOBALSINDEX))
#define lua_pushcfunction(L, f) lua_pushcclosure(L, f, 0)
#define lua_isnoneornil(L, n) (lua_type(L, n) <= 0)
#define lua_isnone(L, n) (lua_type(L, n) == LUA_TNONE)
#define lua_isnil(L, n) (lua_type(L, n) == LUA_TNIL)
#define lua_isboolean(L, n) (lua_type(L, n) == LUA_TBOOLEAN)
#define lua_islightuserdata(L, n) (lua_type(L, n) == LUA_TLIGHTUSERDATA)
#define lua_istable(L, n) (lua_type(L, n) == LUA_TTABLE)
#define lua_iscfunction(L, n) (lua_type(L, n) == LUA_TFUNCTION)
#define lua_isfunction(L, n) (lua_type(L, n) == LUA_TFUNCTION)
#define lua_isuserdata(L, n) (lua_type(L, n) == LUA_TUSERDATA)
#define lua_isthread(L, n) (lua_type(L, n) == LUA_TTHREAD)
#define lua_pushliteral(L, s) lua_pushlstring(L, "" s, (sizeof(s) / sizeof(char)) - 1)
#define lua_getregistry(L) lua_pushvalue(L, LUA_REGISTRYINDEX)
#define lua_setglobal(L, s) (lua_pushstring(L, s), lua_insert(L, -2), lua_settable(L, LUA_GLOBALSINDEX))
#define lua_getglobal(L, s) (lua_pushstring(L, s), lua_gettable(L, LUA_GLOBALSINDEX))
