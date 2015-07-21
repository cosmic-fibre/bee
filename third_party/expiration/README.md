# expirationd -  data expiration with custom quirks.

This package can turn bee into a persistent memcache replacement, 
but is powerful enough so that  your own expiration strategy can be defined.

You define two functions: one takes a tuple as an input and returns
true in case it's expirted and false otherwise. The other takes the 
tuple and performs the expiry itself: either deletes it (memcache), or
does something smarter, like put a smaller representation of the data
being deleted into some other space.

### Example
``` lua
box.cfg{}
space = box.space.old
job_name = 'clean_all'
expirationd = require('expirationd')
function is_expired(args, tuple)
  return true
end
function delete_tuple(space_id, args, tuple)
  box.space[space_id]:delete{tuple[1]}
end
expirationd.run_task(job_name, space.id, is_expired, delete_tuple, nil, 50, 3600)
```
## API

### `expirationd.run_task (name, space_no, is_tuple_expired, process_expired_tuple, args, tuples_per_item, full_scan_time)`
* `name` - task name
* `space_no` - space to look in for expired tuples
* `is_tuple_expired` - a function, must accept tuple and return true/false (is tuple expired or not), receives `(args, tuple)` as arguments
* `process_expired_tuple` - applied to expired tuples, receives `(space_no, args, tuple)` as arguments
* `args` - passed to `is_tuple_expired()` and `process_expired_tuple()` as additional context
* `tuples_per_iter` - number of tuples will be checked by one iteration
* `full_scan_time` - time required for full index scan (in seconds)

### `expirationd.kill_task (name)`
* `name` - task's name

### `expirationd.show_task_list (print_head)`
* `print_head` - print table head

### `expirationd.task_details (task_name)`
* `name` - task's name

## Testing

Simply start `bee test.lua`
