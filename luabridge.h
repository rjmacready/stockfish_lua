#ifndef LUABRIDGE_H_INCLUDED
#define LUABRIDGE_H_INCLUDED


#include "types.h"
#include "position.h"
#include "search.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

void luabridge_close();
void luabridge_loadScripts();
void luabridge_registerSFFunctions();
Value luabridge_evaluate(const Position* pos, Value v);

Move luabridge_pickmove(Search::RootMoveVector rootMoves, size_t multiPV);

#endif