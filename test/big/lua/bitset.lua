local SPACE_NO = 0 
local INDEX_NO = 1

function create_space()
    local space = db.schema.create_space('tweedledum')
    space:create_index('primary', { type = 'hash', parts = {1, 'num'}, unique = true })
    space:create_index('bitset', { type = 'bitset', parts = {2, 'num'}, unique = false })
end

function fill(...)
    local space = db.space['tweedledum']
	local nums = table.generate(arithmetic(...))
	table.shuffle(nums)
	for _k, v in ipairs(nums) do
		space:insert{v, v}
	end
end

function delete(...)
    local space = db.space['tweedledum']
	local nums = table.generate(arithmetic(...))
	table.shuffle(nums)
	for _k, v in ipairs(nums) do
		space:delete{v}
	end
end

function clear()
    db.space['tweedledum']:truncate()
end

function drop_space()
    db.space['tweedledum']:drop()
end

function dump(...)
	return iterate('tweedledum', 'bitset', 1, 2, ...)
end

function test_insert_delete(n)
	local t = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53,
		59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127}

	table.shuffle(t)

	clear()
	fill(1, n)

	for _, v in ipairs(t) do delete(v, n / v) end
	return dump(db.index.BITS_ALL)
end
