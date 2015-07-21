
-- truncate

s = db.schema.space.create('name_of_space', {engine='sham'})
i = s:create_index('name_of_index', {type = 'tree', parts = {1, 'STR'}})
s:insert{'a', 'b', 'c'}
db.space['name_of_space']:select{'a'}
db.space['name_of_space']:truncate()
db.space['name_of_space']:select{'a'}
s:drop()
sham_schedule()
