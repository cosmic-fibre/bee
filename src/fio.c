/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "fio.h"

#include <sys/types.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <lib/bit/bit.h>

#include <say.h>
#include <trivia/util.h>

const char *
fio_filename(int fd)
{
#ifdef TARGET_OS_LINUX
	char proc_path[32];
	static __thread char filename_path[PATH_MAX];

	sprintf(proc_path, "/proc/self/fd/%d", fd);

	ssize_t sz = readlink(proc_path, filename_path,
			      sizeof(filename_path));

	if (sz >= 0) {
		filename_path[sz] = '\0';
		return filename_path;
	}
#else /* TARGET_OS_LINUX */
	(void) fd;
#endif
	return ""; /* Not implemented. */
}

ssize_t
fio_read(int fd, void *buf, size_t count)
{
	ssize_t to_read = (ssize_t) count;
	while (to_read > 0) {
		ssize_t nrd = read(fd, buf, to_read);
		if (nrd < 0) {
			if (errno == EINTR) {
				errno = 0;
				continue;
			}
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return count != to_read ? count - to_read : -1;
			say_syserror("read, [%s]", fio_filename(fd));
			return -1; /* XXX: file position is unspecified */
		}
		if (nrd == 0)
			break;

		buf += nrd;
		to_read -= nrd;
	}
	return count - to_read;
}

ssize_t
fio_write(int fd, const void *buf, size_t count)
{
	ssize_t to_write = (ssize_t) count;
	while (to_write > 0) {
		ssize_t nwr = write(fd, buf, to_write);
		if (nwr < 0) {
			if (errno == EINTR) {
				errno = 0;
				continue;
			}
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return count != to_write ? count - to_write : -1;
			say_syserror("write, [%s]", fio_filename(fd));
			return -1; /* XXX: file position is unspecified */
		}
		if (nwr == 0)
			break;

		buf += nwr;
		to_write -= nwr;
	}
	return count - to_write;
}


ssize_t
fio_writev(int fd, struct iovec *iov, int iovcnt)
{
	assert(iov && iovcnt >= 0);
	ssize_t nwr;
restart:
	nwr = writev(fd, iov, iovcnt);
	if (nwr < 0) {
		if (errno == EINTR) {
			errno = 0;
			goto restart;
		}
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			say_syserror("writev, [%s]", fio_filename(fd));
	}
	return nwr;
}

off_t
fio_lseek(int fd, off_t offset, int whence)
{
	off_t effective_offset = lseek(fd, offset, whence);

	if (effective_offset == -1) {
		say_syserror("lseek, [%s]: offset=%jd, whence=%d",
			     fio_filename(fd), (intmax_t) offset, whence);
	} else if (whence == SEEK_SET && effective_offset != offset) {
		say_error("lseek, [%s]: offset set to unexpected value: "
			  "requested %jd effective %jd",
			  fio_filename(fd),
			  (intmax_t)offset, (intmax_t)effective_offset);
	}
	return effective_offset;
}

int
fio_truncate(int fd, off_t offset)
{
	int rc = ftruncate(fd, offset);
	if (rc)
		say_syserror("fio_truncate, [%s]: offset=%jd",
			     fio_filename(fd), (intmax_t) offset);
	return rc;
}

struct fio_batch *
fio_batch_alloc(int max_iov)
{
	struct fio_batch *batch = (struct fio_batch *)
		malloc(sizeof(struct fio_batch) +
		       sizeof(struct iovec) * max_iov +
		       (max_iov / CHAR_BIT + 1));
	if (batch == NULL)
		return NULL;
	batch->bytes = batch->rows = batch->iovcnt = batch->max_rows = 0;
	batch->max_iov = max_iov;
	batch->rowflag = (char *) (batch + 1) + sizeof(struct iovec) * max_iov;
	return batch;
}

void
fio_batch_start(struct fio_batch *batch, long max_rows)
{
	batch->bytes = batch->rows = batch->iovcnt = 0;
	batch->max_rows = max_rows;
	memset(batch->rowflag, 0, batch->max_iov / CHAR_BIT + 1);
}

void
fio_batch_add(struct fio_batch *batch, const struct iovec *iov, int iovcnt)
{
	assert(!fio_batch_has_space(batch, iovcnt));
	assert(iovcnt > 0);
	assert(batch->max_rows > 0);
	for (int i = 0; i < iovcnt; i++) {
		batch->iov[batch->iovcnt++] = iov[i];
		batch->bytes += iov[i].iov_len;
	}
	bit_set(batch->rowflag, batch->iovcnt);
	batch->rows++;
}

int
fio_batch_write(struct fio_batch *batch, int fd)
{
	ssize_t bytes_written = fio_writev(fd, batch->iov, batch->iovcnt);
	if (bytes_written <= 0)
		return 0;

	if (bytes_written == batch->bytes)
		return batch->rows; /* returns the number of written rows */

	say_warn("fio_batch_write, [%s]: partial write,"
		 " wrote %jd out of %jd bytes",
		 fio_filename(fd),
		 (intmax_t) bytes_written, (intmax_t) batch->bytes);

	/* Iterate over end of row flags */
	struct bit_iterator bit_it;
	bit_iterator_init(&bit_it, batch->rowflag,
			  batch->max_iov / CHAR_BIT + 1, 1);
	size_t row_last_iov = bit_iterator_next(&bit_it);

	int good_rows = 0; /* the number of fully written rows */
	ssize_t good_bytes = 0; /* the number of bytes in fully written rows */
	ssize_t row_bytes = 0;  /* the number of bytes in the current row */
	struct iovec *iov = batch->iov;
	while (iov < batch->iov + batch->iovcnt) {
		if (good_bytes + row_bytes + iov->iov_len > bytes_written)
			break;
		row_bytes += iov->iov_len;
		if ((iov - batch->iov) == row_last_iov) {
			/* the end of current row  */
			good_bytes += row_bytes;
			row_bytes = 0;
			good_rows++;
			row_last_iov = bit_iterator_next(&bit_it);
		}
		iov++;
	}
	/*
	 * Unwind file position back to ensure we do not leave
	 * partially written rows.
	 */
	off_t good_offset = fio_lseek(fd,
				      good_bytes - bytes_written, SEEK_CUR);
	/*
	 * The caller may choose to close the file right after
	 * a partial write. Don't take chances and make sure that
	 * there is no garbage at the end of file if it happens.
	 */
	if (good_offset != -1)
		(void) fio_truncate(fd, good_offset);
	/*
	 * writev() doesn't set errno in case of a partial write.
	 * If nothing else from the above failed, set errno to
	 * EAGAIN.
	 */
	if (! errno)
		errno = EAGAIN;
	return good_rows;  /* returns the number of written rows */
}
