
function string_function()
	local random_number
	local random_string
	random_string = ""
	for x = 1,20,1 do
		random_number = math.random(65, 90)
		random_string = random_string .. string.char(random_number)
	end
	return random_string
end

function delete_replace_update(engine_name)
	local string_value
	if (db.space._space.index.name:select{'tester'}[1] ~= nil) then
		db.space.tester:drop()
	end
	db.schema.space.create('tester', {engine=engine_name})
	db.space.tester:create_index('primary',{type = 'tree', parts = {2, 'STR'}})

	counter = 1
	while counter < 100000 do
		string_value = string_function()

		string_table = db.space.tester.index.primary:select({string_value}, {iterator = 'GE', limit = 1})
		if string_table[1] == nil then
			db.space.tester:insert{counter, string_value}
			string_value_2 = string_value
		else
			string_value_2 = string_table[1][2]
		end

		if string_value_2 == nil then
			db.space.tester:insert{counter, string_value}
			string_value_2 = string_value
		end

		random_number = math.random(1,6)

		string_value_3 = string_function()
--		print('<'..counter..'> [' ..  random_number .. '] value_2: ' .. string_value_2 .. ' value_3: ' .. string_value_3)
		if random_number == 1 then
			db.space.tester:delete{string_value_2}
		end
		if random_number == 2 then
			db.space.tester:replace{counter, string_value_2, string_value_3}
		end
		if random_number == 3 then
			db.space.tester:delete{string_value_2}
			db.space.tester:insert{counter, string_value_2}
		end
		if random_number == 4 then
			if counter < 1000000 then
				db.space.tester:delete{string_value_3}
				db.space.tester:insert{counter, string_value_3, string_value_2}
			end
		end
		if random_number == 5 then
			db.space.tester:update({string_value_2}, {{'=', 1, string_value_3}})
		end
		if random_number == 6 then
			db.space.tester:update({string_value_2}, {{'=', 1, string_value_3}})
		end
		counter = counter + 1
	end

	db.space.tester:drop()
	return {counter, random_number, string_value_2, string_value_3}
end

function delete_insert(engine_name)
	local string_value
	if (db.space._space.index.name:select{'tester'}[1] ~= nil) then
		db.space.tester:drop()
	end
	db.schema.space.create('tester', {engine=engine_name})
	db.space.tester:create_index('primary',{type = 'tree', parts = {2, 'STR'}})
	counter = 1
	while counter < 100000 do
		string_value = string_function()
		string_table = db.space.tester.index.primary:select({string_value}, {iterator = 'GE', limit = 1})

		if string_table[1] == nil then
			-- print (1, ' insert', counter, string_value)
			db.space.tester:insert{counter, string_value}
			string_value_2 = string_value
		else
			string_value_2 = string_table[1][2]
		end

		if string_value_2 == nil then
			-- print (2, ' insert', counter, string_value)
			db.space.tester:insert{counter, string_value}
			string_value_2 = string_value
		end

		-- print (3, ' delete', counter, string_value_2)
		db.space.tester:delete{string_value_2}

		-- print (4, ' insert', counter, string_value_2)
		db.space.tester:insert{counter, string_value_2}

		counter = counter + 1
	end
	db.space.tester:drop()
	return {counter, string_value_2}
end
