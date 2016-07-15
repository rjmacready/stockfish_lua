#include <algorithm>
#include <cmath>
#include "luabridge.h"
#include "uci.h"

lua_State *L = NULL;
char* LuaScript_Path = NULL;

void luabridge_close()
{
	if (L != NULL)
	{
		lua_close(L);
		L = NULL;
	}
}

void luabridge_loadScripts()
{
#if LOGS
	fprintf(stderr, "luabridge_loadScripts\n");
#endif
	int luaerror = 0;

	luabridge_close();

	if (LuaScript_Path == NULL || strcmp(LuaScript_Path, "<empty>") == 0) return;

	L = luaL_newstate();
	luaL_openlibs(L);

	luabridge_registerSFFunctions();

	if (luaerror = luaL_dofile(L, LuaScript_Path))
	{
#if LOGS
		fprintf(stderr, "Some error running lua code: %d %s\n", luaerror, lua_tostring(L, -1));
#endif
	}
}

int luabridge_sideToMove(lua_State* L)
{
	Position* pos = (Position*)lua_topointer(L, 1);	
	lua_pushinteger(L, pos->side_to_move());
	return 1;
}

int luabridge_fen(lua_State* L)
{
	Position* pos = (Position*)lua_topointer(L, 1);
	lua_pushstring(L, pos->fen().c_str());
	return 1;
}

void luabridge_registerSFFunctions()
{
	lua_pushcfunction(L, luabridge_sideToMove);
	lua_setglobal(L, "sideToMove");
	lua_pushcfunction(L, luabridge_fen);
	lua_setglobal(L, "fen");
}

Value luabridge_evaluate(const Position* pos, Value v) 
{	
	return v;

	Value tmp = v;
	if (L != NULL)
	{
		lua_getglobal(L, "evaluate");
		lua_pushlightuserdata(L, (void*)pos);
		lua_pushinteger(L, v);
		lua_pcall(L, 2, 1, 0);

		if (lua_isnumber(L, -1))
		{
			tmp = (Value)lua_tointeger(L, -1);
		}
		lua_pop(L, 1);

#if LOGS
		fprintf(stderr, "luabridge_evaluate (%d) = Result is %d\n", v, tmp);
#endif
	}
	return tmp;
}

Move luabridge_pickmove(Search::RootMoveVector rootMoves, size_t multiPV)
{
	Move tmp = rootMoves.size() > 0 && rootMoves[0].pv.size() > 0 ? rootMoves[0].pv[0] : MOVE_NULL;
	int lua_err;
	lua_getglobal(L, "pickmove");
		
	// transform to lua data structure
	lua_newtable(L);
	
	int idx = 0;
	for (int i = 0; i < std::min(multiPV, rootMoves.size()); ++i)
	{
		Search::RootMove mov = rootMoves[i];
		Value v = mov.score;
		int av = abs(v);

		if (av == VALUE_INFINITE || av == VALUE_NONE) continue;

		lua_pushnumber(L, idx + 1);
		++idx;
		
		lua_newtable(L);
				
		if (av < VALUE_MATE - MAX_PLY)
		{
			lua_pushliteral(L, "score_type");
			lua_pushstring(L, "cp");
			lua_settable(L, -3);

			lua_pushliteral(L, "score");
			lua_pushnumber(L, v * 100 / PawnValueEg);
			lua_settable(L, -3);
		}
		else
		{
			lua_pushliteral(L, "score_type");
			lua_pushstring(L, "mate");
			lua_settable(L, -3);

			lua_pushliteral(L, "score");
			lua_pushnumber(L, (v > 0 ? VALUE_MATE - v + 1 : -VALUE_MATE - v) / 2);
			lua_settable(L, -3);
		}

		
		lua_pushliteral(L, "pv");
		//lua_pushnumber(L, 1);

		lua_newtable(L);
		for (int y = 0; y < mov.pv.size(); ++y)
		{
			lua_pushnumber(L, y + 1);

			lua_newtable(L);

			lua_pushliteral(L, "raw");
			lua_pushinteger(L, mov.pv[y]);
			lua_settable(L, -3);

			lua_pushliteral(L, "move");
			lua_pushstring(L, UCI::move(mov.pv[y], false).c_str());

			lua_settable(L, -3);

			lua_settable(L, -3);
		}

		lua_settable(L, -3);

		lua_settable(L, -3);
	}
	
	int err = lua_pcall(L, 1, 1, 0);
	if (err != 0)
	{
#if LOGS
		fprintf(stderr, "Some error running lua code: %d %s\n", err, lua_tostring(L, -1));
#endif
	}

	if (lua_istable(L, -1))
	{
		lua_pushstring(L, "pv");
		lua_gettable(L, -2);

		if (!lua_istable(L, -1))
		{

		}
		lua_pushnumber(L, 0);
		lua_gettable(L, -2);

		if (!lua_istable(L, -1))
		{

		}
		lua_pushstring(L, "raw");
		lua_gettable(L, -2);
		if (lua_isinteger(L, -1))
		{
			tmp = (Move)lua_tointeger(L, -1);
		}
		lua_pop(L, 3);

/*
		lua_err = lua_getfield(L, -1, "pv");
		if (lua_err != 0)
		{
			fprintf(stderr, "Some error running lua code: %d %s\n", lua_err, lua_tostring(L, -1));
		}
		lua_err = lua_getfield(L, -2, "0");
		if (lua_err != 0)
		{
			fprintf(stderr, "Some error running lua code: %d %s\n", lua_err, lua_tostring(L, -1));
		}
		lua_err = lua_getfield(L, -3, "raw");
		if (lua_err != 0)
		{
			fprintf(stderr, "Some error running lua code: %d %s\n", lua_err, lua_tostring(L, -1));
		}
		if (lua_isinteger(L, -4))
		{
			tmp = (Move)lua_tointeger(L, -4);			
		}		
		lua_pop(L, 3);	*/	
	}
	else if (lua_isinteger(L, -1))
	{
		tmp = (Move)lua_tointeger(L, -1);
	}
	lua_pop(L, 1);
	
#if LOGS
	fprintf(stderr, "Move selected from lua is %s\n", UCI::move(tmp, false).c_str());
	fprintf(stderr, "Move selected initial is %s\n", UCI::move(rootMoves[0].pv[0], false).c_str());
#endif
	return tmp;
}

