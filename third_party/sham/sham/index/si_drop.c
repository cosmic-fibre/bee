
/*
 * sham database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <libsr.h>
#include <libsv.h>
#include <libsd.h>
#include <libsi.h>

static inline int
si_dropof(siconf *conf, sr *r)
{
	DIR *dir = opendir(conf->path);
	if (dir == NULL) {
		sr_malfunction(r->e, "directory '%s' open error: %s",
		               conf->path, strerror(errno));
		return -1;
	}
	char path[1024];
	int rc;
	struct dirent *de;
	while ((de = readdir(dir))) {
		if (de->d_name[0] == '.')
			continue;
		/* skip drop file */
		if (srunlikely(strcmp(de->d_name, "drop") == 0))
			continue;
		snprintf(path, sizeof(path), "%s/%s", conf->path, de->d_name);
		rc = sr_fileunlink(path);
		if (srunlikely(rc == -1)) {
			sr_malfunction(r->e, "db file '%s' unlink error: %s",
			               path, strerror(errno));
			closedir(dir);
			return -1;
		}
	}
	closedir(dir);

	snprintf(path, sizeof(path), "%s/drop", conf->path);
	rc = sr_fileunlink(path);
	if (srunlikely(rc == -1)) {
		sr_malfunction(r->e, "db file '%s' unlink error: %s",
		               path, strerror(errno));
		return -1;
	}
	rc = rmdir(conf->path);
	if (srunlikely(rc == -1)) {
		sr_malfunction(r->e, "directory '%s' unlink error: %s",
		               conf->path, strerror(errno));
		return -1;
	}
	return 0;
}

static int
si_dropmark(si *i, sr *r)
{
	/* create drop file */
	char path[1024];
	snprintf(path, sizeof(path), "%s/drop", i->conf->path);
	srfile drop;
	sr_fileinit(&drop, r->a);
	int rc = sr_filenew(&drop, path);
	if (srunlikely(rc == -1)) {
		sr_malfunction(r->e, "drop file '%s' create error: %s",
		               path, strerror(errno));
		return -1;
	}
	sr_fileclose(&drop);
	return 0;
}

int si_drop(si *i, sr *r)
{
	siconf *conf = i->conf;
	/* drop file must exists at this point */
	/* shutdown */
	int rc = si_close(i, r);
	if (srunlikely(rc == -1))
		return -1;
	if (! i->dropped) {
		rc = si_dropmark(i, r);
		if (srunlikely(rc == -1))
			return -1;
	}
	/* remove directory */
	rc = si_dropof(conf, r);
	return rc;
}
