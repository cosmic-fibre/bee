#!/usr/bin/env bee

local os = require('os')

local ffi = require('ffi')
ffi.cdef[[
	int sham_schedule(void);
]]

function sham_schedule()
	ffi.C.sham_schedule()
end

function sham_dir()
	local i = 0
	local list = {}
	for file in io.popen("ls -1 sham_test"):lines() do
		i = i + 1
		list[i] = file
	end
	return {i, t}
end

function sham_mkdir(dir)
	os.execute("mkdir sham_test")
end

function sham_rmdir(dir)
	os.execute("rm -rf sham_test")
end

function file_exists(name)
	local f = io.open(name,"r")
	if f ~= nil then
		io.close(f)
		return true
	else
		return false
	end
end
