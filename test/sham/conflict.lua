
function test_conflict()
	s = db.schema.space.create('tester', {engine='sham'});
	i = s:create_index('sham_index', {type = 'tree', parts = {1, 'STR'}});

	commits = 0
	function conflict()
		db.begin()
		s:replace({'test'})
		db.commit()
		commits = commits + 1
	end;

	fiber = require('fiber');
	f0 = fiber.create(conflict);
	f1 = fiber.create(conflict); -- conflict
	fiber.sleep(0);

	s:drop();
	sham_schedule();
	return commits
end
