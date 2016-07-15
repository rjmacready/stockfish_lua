-- require "table2"
-- require "string2"
file = io.open("log.txt", "w") 
-- io.output(file)
cp_error_margin = 100.0
cp_no_more_error_margin = -300.0

function printline(smth)
	
	--io.
	file:write(smth)
	--io.
	file:write("\n")
	--io.
	file:flush()
end

-- print("*log* Hello World from lua")

printline("Hello from lua world")

function _evaluate(pos, a)
	s = sideToMove(pos)
	-- print("side to move is ")
	-- print(s)
	
	-- print("fen is " .. fen(pos))
	return a * -1
end


function print_moves(moves)
	printline(" pick moves with \n");
	for key, value in pairs(moves) do
		printline(key)
		
		for key2, value2 in pairs(value) do
			printline("    " .. key2)
			
			
			if type (value2) == "table" then				
				printline("  		" .. value2[1]["move"])
				--for key3, value3 in pairs(value2) do
					--print("  [" .. key3 .. "] ")
					
					--for key4, value4 in pairs(value3) do
					--	print("		" .. key4)
					--	print("		" .. value4)
					--end
				--end
			else
				printline("    " .. value2)
			end
		end
		
	end

end

function pickmove(moves)
	if #tb == 0 then
		return 0;
	end
	
	print_moves(moves);
	printline("cp_no_more_error_margin: " .. cp_no_more_error_margin);
	printline("cp_error_margin: " .. cp_error_margin);
	
	local bestScore = moves[1].score;
	printline("bestScore is " .. bestScore);
	
	if bestScore< cp_no_more_error_margin then
		printline("is losing, return best move " .. moves[1].pv[1].move);
		return moves[1].pv[1].raw;
	end
	
	local tb = {}
	local k2 = 1
	for k,v in pairs(moves) do
		if v.score_type == "mate" then
			printline("has mate, return " ..  v.pv[1].move)
			return v.pv[1].raw
		end
		
		if v.score_type == "cp" and v.score >= bestScore - cp_error_margin then 
			tb[k2] = v
			k2 = k2 + 1
		end
	end
	
	print_moves(tb);
	
	-- pick at random
	local move = tb[math.random(#tb)].pv[1]
	printline("not losing, return " .. move.move);
	return move.raw
	
	
	-- if (# moves) >= 2 then
	--	return moves[1].pv[0].raw;
	--end
	
	-- TODO allow different material evaluation?
	-- TODO add noise to eval?
	-- TODO pick move at random from within margin of error, drop that margin if self is losing
		
	-- return moves[0].pv[0].raw;
end