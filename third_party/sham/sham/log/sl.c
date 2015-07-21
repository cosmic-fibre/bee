
/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <libsr.h>
#include <libsv.h>
#include <libsl.h>

static inline sl*
sl_alloc(slpool *p, uint32_t id)
{
	sl *l = sr_malloc(p->r->a, sizeof(*l));
	if (srunlikely(l == NULL)) {
		sr_malfunction(p->r->e, "%s", "memory allocation failed");
		return NULL;
	}
	l->id   = id;
	l->p    = NULL;
	sr_gcinit(&l->gc);
	sr_mutexinit(&l->filelock);
	sr_fileinit(&l->file, p->r->a);
	sr_listinit(&l->link);
	sr_listinit(&l->linkcopy);
	return l;
}

static inline int
sl_close(slpool *p, sl *l)
{
	int rc = sr_fileclose(&l->file);
	if (srunlikely(rc == -1)) {
		sr_malfunction(p->r->e, "log file '%s' close error: %s",
		               l->file.file, strerror(errno));
	}
	sr_mutexfree(&l->filelock);
	sr_gcfree(&l->gc);
	sr_free(p->r->a, l);
	return rc;
}

static inline sl*
sl_open(slpool *p, uint32_t id)
{
	sl *l = sl_alloc(p, id);
	if (srunlikely(l == NULL))
		return NULL;
	srpath path;
	sr_pathA(&path, p->conf->path, id, ".log");
	int rc = sr_fileopen(&l->file, path.path);
	if (srunlikely(rc == -1)) {
		sr_malfunction(p->r->e, "log file '%s' open error: %s",
		               l->file.file, strerror(errno));
		goto error;
	}
	return l;
error:
	sl_close(p, l);
	return NULL;
}

static inline sl*
sl_new(slpool *p, uint32_t id)
{
	sl *l = sl_alloc(p, id);
	if (srunlikely(l == NULL))
		return NULL;
	srpath path;
	sr_pathA(&path, p->conf->path, id, ".log");
	int rc = sr_filenew(&l->file, path.path);
	if (srunlikely(rc == -1)) {
		sr_malfunction(p->r->e, "log file '%s' create error: %s",
		               path.path, strerror(errno));
		goto error;
	}
	srversion v;
	sr_version(&v);
	rc = sr_filewrite(&l->file, &v, sizeof(v));
	if (srunlikely(rc == -1)) {
		sr_malfunction(p->r->e, "log file '%s' header write error: %s",
		               l->file.file, strerror(errno));
		goto error;
	}
	return l;
error:
	sl_close(p, l);
	return NULL;
}

int sl_poolinit(slpool *p, sr *r)
{
	sr_spinlockinit(&p->lock);
	sr_listinit(&p->list);
	p->n    = 0;
	p->r    = r;
	p->gc   = 1;
	p->conf = NULL;
	struct iovec *iov =
		sr_malloc(r->a, sizeof(struct iovec) * 1021);
	if (srunlikely(iov == NULL))
		return sr_malfunction(r->e, "%s", "memory allocation failed");
	sr_iovinit(&p->iov, iov, 1021);
	return 0;
}

static inline int
sl_poolcreate(slpool *p)
{
	int rc;
	rc = sr_filemkdir(p->conf->path);
	if (srunlikely(rc == -1))
		return sr_malfunction(p->r->e, "log directory '%s' create error: %s",
		                      p->conf->path, strerror(errno));
	return 1;
}

static inline int
sl_poolrecover(slpool *p)
{
	srbuf list;
	sr_bufinit(&list);
	srdirtype types[] =
	{
		{ "log", 1, 0 },
		{ NULL,  0, 0 }
	};
	int rc = sr_dirread(&list, p->r->a, types, p->conf->path);
	if (srunlikely(rc == -1))
		return sr_malfunction(p->r->e, "log directory '%s' open error",
		                      p->conf->path);
	sriter i;
	sr_iterinit(sr_bufiter, &i, p->r);
	sr_iteropen(sr_bufiter, &i, &list, sizeof(srdirid));
	while(sr_iterhas(sr_bufiter, &i)) {
		srdirid *id = sr_iterof(sr_bufiter, &i);
		sl *l = sl_open(p, id->id);
		if (srunlikely(l == NULL)) {
			sr_buffree(&list, p->r->a);
			return -1;
		}
		sr_listappend(&p->list, &l->link);
		p->n++;
		sr_iternext(sr_bufiter, &i);
	}
	sr_buffree(&list, p->r->a);
	if (p->n) {
		sl *last = srcast(p->list.prev, sl, link);
		p->r->seq->lfsn = last->id;
		p->r->seq->lfsn++;
	}
	return 0;
}

int sl_poolopen(slpool *p, slconf *conf)
{
	p->conf = conf;
	if (srunlikely(! p->conf->enable))
		return 0;
	int exists = sr_fileexists(p->conf->path);
	int rc;
	if (! exists)
		rc = sl_poolcreate(p);
	else
		rc = sl_poolrecover(p);
	if (srunlikely(rc == -1))
		return -1;
	return 0;
}

int sl_poolrotate(slpool *p)
{
	if (srunlikely(! p->conf->enable))
		return 0;
	uint32_t lfsn = sr_seq(p->r->seq, SR_LFSNNEXT);
	sl *l = sl_new(p, lfsn);
	if (srunlikely(l == NULL))
		return -1;
	sl *log = NULL;
	sr_spinlock(&p->lock);
	if (p->n) {
		log = srcast(p->list.prev, sl, link);
		sr_gccomplete(&log->gc);
	}
	sr_listappend(&p->list, &l->link);
	p->n++;
	sr_spinunlock(&p->lock);
	if (log) {
		if (p->conf->sync_on_rotate) {
			int rc = sr_filesync(&log->file);
			if (srunlikely(rc == -1)) {
				sr_malfunction(p->r->e, "log file '%s' sync error: %s",
				               log->file.file, strerror(errno));
				return -1;
			}
		}
	}
	return 0;
}

int sl_poolrotate_ready(slpool *p, int wm)
{
	if (srunlikely(! p->conf->enable))
		return 0;
	sr_spinlock(&p->lock);
	assert(p->n > 0);
	sl *l = srcast(p->list.prev, sl, link);
	int ready = sr_gcrotateready(&l->gc, wm);
	sr_spinunlock(&p->lock);
	return ready;
}

int sl_poolshutdown(slpool *p)
{
	int rcret = 0;
	int rc;
	if (p->n) {
		srlist *i, *n;
		sr_listforeach_safe(&p->list, i, n) {
			sl *l = srcast(i, sl, link);
			rc = sl_close(p, l);
			if (srunlikely(rc == -1))
				rcret = -1;
		}
	}
	if (p->iov.v)
		sr_free(p->r->a, p->iov.v);
	sr_spinlockfree(&p->lock);
	return rcret;
}

static inline int
sl_gc(slpool *p, sl *l)
{
	int rc;
	rc = sr_fileunlink(l->file.file);
	if (srunlikely(rc == -1)) {
		return sr_malfunction(p->r->e, "log file '%s' unlink error: %s",
		                      l->file.file, strerror(errno));
	}
	rc = sl_close(p, l);
	if (srunlikely(rc == -1))
		return -1;
	return 1;
}

int sl_poolgc_enable(slpool *p, int enable)
{
	sr_spinlock(&p->lock);
	p->gc = enable;
	sr_spinunlock(&p->lock);
	return 0;
}

int sl_poolgc(slpool *p)
{
	if (srunlikely(! p->conf->enable))
		return 0;
	for (;;) {
		sr_spinlock(&p->lock);
		if (srunlikely(! p->gc)) {
			sr_spinunlock(&p->lock);
			return 0;
		}
		sl *current = NULL;
		srlist *i;
		sr_listforeach(&p->list, i) {
			sl *l = srcast(i, sl, link);
			if (srlikely(! sr_gcgarbage(&l->gc)))
				continue;
			sr_listunlink(&l->link);
			p->n--;
			current = l;
			break;
		}
		sr_spinunlock(&p->lock);
		if (current) {
			int rc = sl_gc(p, current);
			if (srunlikely(rc == -1))
				return -1;
		} else {
			break;
		}
	}
	return 0;
}

int sl_poolfiles(slpool *p)
{
	sr_spinlock(&p->lock);
	int n = p->n;
	sr_spinunlock(&p->lock);
	return n;
}

int sl_poolcopy(slpool *p, char *dest, srbuf *buf)
{
	srlist list;
	sr_listinit(&list);
	sr_spinlock(&p->lock);
	srlist *i;
	sr_listforeach(&p->list, i) {
		sl *l = srcast(i, sl, link);
		if (sr_gcinprogress(&l->gc))
			break;
		sr_listappend(&list, &l->linkcopy);
	}
	sr_spinunlock(&p->lock);

	sr_bufinit(buf);
	srlist *n;
	sr_listforeach_safe(&list, i, n)
	{
		sl *l = srcast(i, sl, linkcopy);
		sr_listinit(&l->linkcopy);
		srpath path;
		sr_pathA(&path, dest, l->id, ".log");
		srfile file;
		sr_fileinit(&file, p->r->a);
		int rc = sr_filenew(&file, path.path);
		if (srunlikely(rc == -1)) {
			sr_malfunction(p->r->e, "log file '%s' create error: %s",
			               path.path, strerror(errno));
			return -1;
		}
		rc = sr_bufensure(buf, p->r->a, l->file.size);
		if (srunlikely(rc == -1)) {
			sr_malfunction(p->r->e, "%s", "memory allocation failed");
			sr_fileclose(&file);
			return -1;
		}
		rc = sr_filepread(&l->file, 0, buf->s, l->file.size);
		if (srunlikely(rc == -1)) {
			sr_malfunction(p->r->e, "log file '%s' read error: %s",
			               l->file.file, strerror(errno));
			sr_fileclose(&file);
			return -1;
		}
		sr_bufadvance(buf, l->file.size);
		rc = sr_filewrite(&file, buf->s, l->file.size);
		if (srunlikely(rc == -1)) {
			sr_malfunction(p->r->e, "log file '%s' write error: %s",
			               path.path, strerror(errno));
			sr_fileclose(&file);
			return -1;
		}
		/* sync? */
		rc = sr_fileclose(&file);
		if (srunlikely(rc == -1)) {
			sr_malfunction(p->r->e, "log file '%s' close error: %s",
			               path.path, strerror(errno));
			return -1;
		}
		sr_bufreset(buf);
	}
	return 0;
}

int sl_begin(slpool *p, sltx *t)
{
	memset(t, 0, sizeof(*t));
	sr_spinlock(&p->lock);
	t->p = p;
	if (! p->conf->enable)
		return 0;
	assert(p->n > 0);
	sl *l = srcast(p->list.prev, sl, link);
	sr_mutexlock(&l->filelock);
	t->svp = sr_filesvp(&l->file);
	t->l = l;
	t->p = p;
	return 0;
}

int sl_commit(sltx *t)
{
	if (t->p->conf->enable)
		sr_mutexunlock(&t->l->filelock);
	sr_spinunlock(&t->p->lock);
	return 0;
}

int sl_rollback(sltx *t)
{
	int rc = 0;
	if (t->p->conf->enable) {
		rc = sr_filerlb(&t->l->file, t->svp);
		if (srunlikely(rc == -1))
			sr_malfunction(t->p->r->e, "log file '%s' truncate error: %s",
			               t->l->file.file, strerror(errno));
		sr_mutexunlock(&t->l->filelock);
	}
	sr_spinunlock(&t->p->lock);
	return rc;
}

static inline int
sl_follow(slpool *p, uint64_t lsn)
{
	sr_seqlock(p->r->seq);
	if (lsn > p->r->seq->lsn)
		p->r->seq->lsn = lsn;
	sr_sequnlock(p->r->seq);
	return 0;
}

int sl_prepare(slpool *p, svlog *vlog, uint64_t lsn)
{
	if (srlikely(lsn == 0))
		lsn = sr_seq(p->r->seq, SR_LSNNEXT);
	else
		sl_follow(p, lsn);
	sriter i;
	sr_iterinit(sr_bufiter, &i, NULL);
	sr_iteropen(sr_bufiter, &i, &vlog->buf, sizeof(svlogv));
	for (; sr_iterhas(sr_bufiter, &i); sr_iternext(sr_bufiter, &i))
	{
		svlogv *v = sr_iterof(sr_bufiter, &i);
		sv_lsnset(&v->v, lsn);
	}
	return 0;
}

static inline void
sl_write_prepare(slpool *p, sltx *t, slv *lv, svlogv *logv)
{
	sv *v = &logv->v;
	lv->lsn     = sv_lsn(v);
	lv->dsn     = logv->id;
	lv->flags   = sv_flags(v);
	lv->size    = sv_size(v);
	lv->reserve = 0;
	lv->crc     = sr_crcp(p->r->crc, sv_pointer(v), lv->size, 0);
	lv->crc     = sr_crcs(p->r->crc, lv, sizeof(slv), lv->crc);
	sr_iovadd(&p->iov, lv, sizeof(slv));
	sr_iovadd(&p->iov, sv_pointer(v), lv->size);
	((svv*)v->v)->log = t->l;
}

static inline int
sl_write_stmt(sltx *t, svlog *vlog)
{
	slpool *p = t->p;
	slv lv;
	svlogv *logv = (svlogv*)vlog->buf.s;
	sl_write_prepare(t->p, t, &lv, logv);
	int rc = sr_filewritev(&t->l->file, &p->iov);
	if (srunlikely(rc == -1)) {
		sr_malfunction(p->r->e, "log file '%s' write error: %s",
		               t->l->file.file, strerror(errno));
		return -1;
	}
	sr_gcmark(&t->l->gc, 1);
	sr_iovreset(&p->iov);
	return 0;
}

static int
sl_write_multi_stmt(sltx *t, svlog *vlog, uint64_t lsn)
{
	slpool *p = t->p;
	sl *l = t->l;
	slv lvbuf[510]; /* 1 + 510 per syscall */
	int lvp;
	int rc;
	lvp = 0;
	/* transaction header */
	slv *lv = &lvbuf[0];
	lv->lsn     = lsn;
	lv->dsn     = 0;
	lv->flags   = SVBEGIN;
	lv->size    = sv_logcount(vlog);
	lv->reserve = 0;
	lv->crc     = sr_crcs(p->r->crc, lv, sizeof(slv), 0);
	sr_iovadd(&p->iov, lv, sizeof(slv));
	lvp++;
	/* body */
	sriter i;
	sr_iterinit(sr_bufiter, &i, p->r);
	sr_iteropen(sr_bufiter, &i, &vlog->buf, sizeof(svlogv));
	for (; sr_iterhas(sr_bufiter, &i); sr_iternext(sr_bufiter, &i))
	{
		if (srunlikely(! sr_iovensure(&p->iov, 2))) {
			rc = sr_filewritev(&l->file, &p->iov);
			if (srunlikely(rc == -1)) {
				sr_malfunction(p->r->e, "log file '%s' write error: %s",
				               l->file.file, strerror(errno));
				return -1;
			}
			sr_iovreset(&p->iov);
			lvp = 0;
		}
		svlogv *logv = sr_iterof(sr_bufiter, &i);
		assert(logv->v.i == &sv_vif);
		lv = &lvbuf[lvp];
		sl_write_prepare(p, t, lv, logv);
		lvp++;
	}
	if (srlikely(sr_iovhas(&p->iov))) {
		rc = sr_filewritev(&l->file, &p->iov);
		if (srunlikely(rc == -1)) {
			sr_malfunction(p->r->e, "log file '%s' write error: %s",
			               l->file.file, strerror(errno));
			return -1;
		}
		sr_iovreset(&p->iov);
	}
	sr_gcmark(&l->gc, sv_logcount(vlog));
	return 0;
}

int sl_write(sltx *t, svlog *vlog)
{
	/* assume transaction log is prepared
	 * (lsn set) */
	if (srunlikely(! t->p->conf->enable))
		return 0;
	int count = sv_logcount(vlog);
	int rc;
	if (srlikely(count == 1)) {
		rc = sl_write_stmt(t, vlog);
	} else {
		svlogv *lv = (svlogv*)vlog->buf.s;
		uint64_t lsn = sv_lsn(&lv->v);
		rc = sl_write_multi_stmt(t, vlog, lsn);
	}
	if (srunlikely(rc == -1))
		return -1;
	/* sync */
	if (t->p->conf->enable && t->p->conf->sync_on_write) {
		rc = sr_filesync(&t->l->file);
		if (srunlikely(rc == -1)) {
			sr_malfunction(t->p->r->e, "log file '%s' sync error: %s",
			               t->l->file.file, strerror(errno));
			return -1;
		}
	}
	return 0;
}
