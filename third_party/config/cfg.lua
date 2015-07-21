shard = require ('shard')


local env = {
	init = function(uri)

		return db.cfg{listen = uri} end,

    basic = function()
    return db.cfg{
    listen = os.getenv("LISTEN_URI"),
    slab_alloc_arena    = 2,
    pid_file            = "cfg_basic.pid",
    rows_per_wal        = 200000
    }
    end,

    ins = function()
    return db.cfg {
    listen = os.getenv("LISTEN_URI"),
    slab_alloc_arena    = 8,
    slab_alloc_maximal  = 3145728,
    pid_file            = "cfg_ins.pid",
    rows_per_wal        = 500000
    }
    end,

    compute = function()
    return db.cfg {
    listen = os.getenv("LISTEN_URI"),
    slab_alloc_arena    = 8,
    slab_alloc_maximal  = 3145728,
    slab_alloc_factor   = 2,
    pid_file            = "cfg_compute.pid",
    rows_per_wal        = 500000
    }
    end,

    user = function(login,passw)


    return db.schema.user.create(login, { password = passw })

    end,

    grant = function(login)

    return db.schema.user.grant(login, 'read,write,execute', 'universe')

	  end,


		replica = function(listen1, replica)
			return db.cfg{listen=listen1, replication_source=replica}
		end

}


return env
