const char expirationd_lua[] =
"-- ========================================================================= --\n"
"-- bee/Box expiration daemon\n"
"--\n"
"-- Daemon management functions:\n"
"--   - expirationd.run_task       -- run a new expiration task\n"
"--   - expirationd.kill_task      -- kill a running task\n"
"--   - expirationd.show_task_list -- print the task list\n"
"--   - expirationd.task_details   -- show task details\n"
"-- ========================================================================= --\n"
"\n"
"-- ========================================================================= --\n"
"-- local support functions\n"
"-- ========================================================================= --\n"
"\n"
"local log = require('log')\n"
"local fiber = require('fiber')\n"
"\n"
"-- Create a new table with constant members. A runtime error\n"
"-- is raised on attempt to change a table field.\n"
"local function finalize_table(table)\n"
"    local str = \"attempt to change constant %q to %q\"\n"
"    return setmetatable ({}, {\n"
"            __index = table,\n"
"            __newindex = function(table_arg,\n"
"                                name_arg,\n"
"                                value_arg)\n"
"                error(str:format(name_arg, value_arg), 2)\n"
"            end\n"
"        })\n"
"end\n"
"\n"
"-- get fiber id function\n"
"local function get_fiber_id(fiber)\n"
"    local fid = 0\n"
"    if fiber ~= nil and fiber:status() ~= \"dead\" then\n"
"        fid = fiber:id()\n"
"    end\n"
"    return fid\n"
"end\n"
"\n"
"-- ========================================================================= --\n"
"-- Expiration daemon global variables\n"
"-- ========================================================================= --\n"
"\n"
"-- main table\n"
"\n"
"local expirationd = {\n"
"    -- task list\n"
"    task_list = {},\n"
"    -- constants\n"
"    constants = finalize_table({\n"
"        -- default value of number of tuples that\n"
"        -- will be checked by one iteration\n"
"        default_tuples_per_iteration = 1024,\n"
"        -- default value of time required for full\n"
"        -- index scan (in seconds)\n"
"        default_full_scan_time = 3600,\n"
"        -- maximal worker delay (seconds)\n"
"        max_delay = 1,\n"
"        -- check worker interval\n"
"        check_interval = 1,\n"
"    }),\n"
"}\n"
"\n"
"-- ========================================================================= --\n"
"-- Task local functions\n"
"-- ========================================================================= --\n"
"\n"
"-- ------------------------------------------------------------------------- --\n"
"-- Task fibers\n"
"-- ------------------------------------------------------------------------- --\n"
"\n"
"local function do_worker_iteration(task)\n"
"    local scan_space = db.space[task.space_id]\n"
"\n"
"    -- full index scan loop\n"
"    local checked_tuples_count = 0\n"
"    for _, tuple in scan_space.index[0]:pairs(nil, {iterator = db.index.ALL}) do\n"
"        checked_tuples_count = checked_tuples_count + 1\n"
"\n"
"        -- do main work\n"
"        if task.is_tuple_expired(task.args, tuple) then\n"
"            task.expired_tuples_count = task.expired_tuples_count + 1\n"
"            task.process_expired_tuple(task.space_id, task.args, tuple)\n"
"        end\n"
"\n"
"        -- find out if the worker can go to sleep\n"
"        if checked_tuples_count >= task.tuples_per_iteration then\n"
"            checked_tuples_count = 0\n"
"            if scan_space:len() > 0 then\n"
"                local delay = (task.tuples_per_iteration * task.full_scan_time) / scan_space:len()\n"
"\n"
"                if delay > expirationd.constants.max_delay then\n"
"                    delay = expirationd.constants.max_delay\n"
"                end\n"
"                fiber.sleep(delay)\n"
"            end\n"
"        end\n"
"    end\n"
"end\n"
"\n"
"local function worker_loop(task)\n"
"    -- detach worker from the guardian and attach it to sched fiber\n"
"    fiber.self():name(task.name)\n"
"\n"
"    while true do\n"
"        if db.cfg.replication_source == nil then\n"
"            do_worker_iteration(task)\n"
"        end\n"
"\n"
"        -- iteration is complete, yield\n"
"        fiber.sleep(expirationd.constants.max_delay)\n"
"    end\n"
"end\n"
"\n"
"local function guardian_loop(task)\n"
"    -- detach the guardian from the creator and attach it to sched\n"
"    fiber.self():name(string.format(\"guardian of %q\", task.name))\n"
"\n"
"    while true do\n"
"        if get_fiber_id(task.worker_fiber) == 0 then\n"
"            -- create worker fiber\n"
"            task.worker_fiber = fiber.create(worker_loop, task)\n"
"\n"
"            log.info(\"expiration: task %q restarted\", task.name)\n"
"            task.restarts = task.restarts + 1\n"
"        end\n"
"        fiber.sleep(expirationd.constants.check_interval)\n"
"    end\n"
"end\n"
"\n"
"\n"
"-- ------------------------------------------------------------------------- --\n"
"-- Task managemet\n"
"-- ------------------------------------------------------------------------- --\n"
"\n"
"-- create new expiration task\n"
"local function create_task(name)\n"
"    local task = {}\n"
"    task.name = name\n"
"    task.start_time = os.time()\n"
"    task.guardian_fiber = nil\n"
"    task.worker_fiber = nil\n"
"    task.space_id = nil\n"
"    task.expired_tuples_count = 0\n"
"    task.restarts = 0\n"
"    task.is_tuple_expired = nil\n"
"    task.process_expired_tuple = nil\n"
"    task.args = nil\n"
"    task.tuples_per_iteration = expirationd.constants.default_tuples_per_iteration\n"
"    task.full_scan_time = expirationd.constants.default_full_scan_time\n"
"    return task\n"
"end\n"
"\n"
"-- get task for table\n"
"local function get_task(name)\n"
"    if name == nil then\n"
"        error(\"task name is nil\")\n"
"    end\n"
"\n"
"    -- check, does the task exist\n"
"    if expirationd.task_list[name] == nil then\n"
"        error(\"task '\" .. name .. \"' doesn't exist\")\n"
"    end\n"
"\n"
"    return expirationd.task_list[name]\n"
"end\n"
"\n"
"-- run task\n"
"local function run_task(task)\n"
"    -- start guardian task\n"
"    task.guardian_fiber = fiber.create(guardian_loop, task)\n"
"end\n"
"\n"
"-- kill task\n"
"local function kill_task(task)\n"
"    if get_fiber_id(task.guardian_fiber) ~= 0 then\n"
"        -- kill guardian fiber\n"
"        fiber.cancel(task.guardian_fiber)\n"
"        task.guardian_fiber = nil\n"
"    end\n"
"    if get_fiber_id(task.worker_fiber) ~= 0 then\n"
"        -- kill worker fiber\n"
"        fiber.cancel(task.worker_fiber)\n"
"        task.worker_fiber = nil\n"
"    end\n"
"end\n"
"\n"
"\n"
"-- ========================================================================= --\n"
"-- Expiration daemon management functions\n"
"-- ========================================================================= --\n"
"\n"
"--\n"
"-- Run a named task\n"
"-- params:\n"
"--    name                  -- task name\n"
"--    space_id              -- space to look in for expired tuples\n"
"--    is_tuple_expired      -- a function, must accept tuple and return\n"
"--                             true/false (is tuple expired or not),\n"
"--                             receives (args, tuple) as arguments\n"
"--    process_expired_tuple -- applied to expired tuples, receives\n"
"--                             (space_id, args, tuple) as arguments\n"
"--    args                  -- passed to is_tuple_expired and process_expired_tuple()\n"
"--                             as additional context\n"
"--    tuples_per_iteration  -- number of tuples will be checked by one iteration\n"
"--    full_scan_time        -- time required for full index scan (in seconds)\n"
"--\n"
"function expirationd.run_task(name,\n"
"                              space_id,\n"
"                              is_tuple_expired,\n"
"                              process_expired_tuple,\n"
"                              args,\n"
"                              tuples_per_iteration,\n"
"                              full_scan_time)\n"
"    if name == nil then\n"
"        error(\"task name is nil\")\n"
"    end\n"
"\n"
"    -- check, does the task exist\n"
"    if expirationd.task_list[name] ~= nil then\n"
"        log.info(\"restart task %q\", name)\n"
"\n"
"        expirationd.kill_task(name)\n"
"    end\n"
"    local task = create_task(name)\n"
"\n"
"    -- required params\n"
"\n"
"    -- check expiration space number (required)\n"
"    if space_id == nil then\n"
"        error(\"space_id is nil\")\n"
"    end\n"
"    task.space_id = space_id\n"
"\n"
"    if is_tuple_expired == nil then\n"
"        error(\"is_tuple_expired is nil, please provide a check function\")\n"
"    elseif type(is_tuple_expired) ~= \"function\" then\n"
"        error(\"is_tuple_expired is not a function, please provide a check function\")\n"
"    end\n"
"    task.is_tuple_expired = is_tuple_expired\n"
"\n"
"    -- process expired tuple handler\n"
"    if process_expired_tuple == nil then\n"
"        error(\"process_expired_tuple is nil, please provide a purge function\")\n"
"    elseif type(process_expired_tuple) ~= \"function\" then\n"
"        error(\"process_expired_tuple is not defined, please provide a purge function\")\n"
"    end\n"
"    task.process_expired_tuple = process_expired_tuple\n"
"\n"
"    -- optional params\n"
"\n"
"    -- check expire and process after expiration handler's arguments\n"
"    task.args = args\n"
"\n"
"    -- check tuples per iteration (not required)\n"
"    if tuples_per_iteration ~= nil then\n"
"        if tuples_per_iteration <= 0 then\n"
"            error(\"invalid tuples per iteration parameter\")\n"
"        end\n"
"        task.tuples_per_iteration = tuples_per_iteration\n"
"    end\n"
"\n"
"    -- check full scan time\n"
"    if full_scan_time ~= nil then\n"
"        if full_scan_time <= 0 then\n"
"            error(\"invalid full scan time\")\n"
"        end\n"
"        task.full_scan_time = full_scan_time\n"
"    end\n"
"\n"
"    --\n"
"    -- run task\n"
"    --\n"
"\n"
"    -- put the task to table\n"
"    expirationd.task_list[name] = task\n"
"    -- run\n"
"    run_task(task)\n"
"end\n"
"\n"
"--\n"
"-- Kill named task\n"
"-- params:\n"
"--    name -- is task's name\n"
"--\n"
"function expirationd.kill_task(name)\n"
"    kill_task(get_task(name))\n"
"    expirationd.task_list[name] = nil\n"
"end\n"
"\n"
"--\n"
"-- Print task list in TSV table format\n"
"-- params:\n"
"--   print_head -- print table head\n"
"--\n"
"function expirationd.show_task_list(print_head)\n"
"    if print_head == nil or print_head == true then\n"
"        log.info('name\\tspace\\texpired\\ttime')\n"
"        log.info('-----------------------------------')\n"
"    end\n"
"    for i, task in pairs(expirationd.task_list) do\n"
"         log.info(\"%q\\t%s\\t%s\\t%f\",\n"
"                  task.name,\n"
"                  task.space_id,\n"
"                  task.expired_tuples_count,\n"
"                  math.floor(os.time() - task.start_time)\n"
"         )\n"
"    end\n"
"end\n"
"\n"
"--\n"
"-- Print task details\n"
"-- params:\n"
"--   name -- task's name\n"
"--\n"
"function expirationd.task_details(name)\n"
"    local task = get_task(name)\n"
"    log.info(\"name: %s\",                          task.name)\n"
"    log.info(\"start time: %f\",                    math.floor(task.start_time))\n"
"    log.info(\"working time: %f\",                  math.floor(os.time() - task.start_time))\n"
"    log.info(\"space: %s\",                         task.space_id)\n"
"    log.info(\"is_tuple_expired handler: %s\",      task.is_tuple_expired)\n"
"    log.info(\"process_expired_tuple handler: %s\", task.process_expired_tuple)\n"
"\n"
"    if task.args ~= nil then\n"
"        log.info(\"args: \")\n"
"        for k, v in pairs(task.args) do\n"
"            log.info(\"\\t%s\\t%s\", k, v)\n"
"        end\n"
"    else\n"
"        log.info(\"args: nil\")\n"
"    end\n"
"    log.info(\"tuples per iteration: %d\", task.tuples_per_iteration)\n"
"    log.info(\"full index scan time: %f\", task.full_scan_time)\n"
"    log.info(\"expired tuples count: %d\", task.expired_tuples_count)\n"
"    log.info(\"restarts: %d\",             task.restarts)\n"
"    log.info(\"guardian fid: %d\",         get_fiber_id(task.guardian_fiber))\n"
"    log.info(\"worker fid: %d\",           get_fiber_id(task.worker_fiber))\n"
"end\n"
"\n"
"return expirationd\n"
""
;
