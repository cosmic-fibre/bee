SHAM_O = so_ctl.o \
           so_ctlcursor.o \
           so_worker.o \
           so_scheduler.o \
           so_request.o \
           so.o \
           so_v.o \
           so_db.o \
           so_tx.o \
           so_cursor.o \
           so_snapshot.o \
           so_snapshotcursor.o \
           so_recover.o \
           sham.o
SHAM_OBJECTS = $(addprefix sham/, $(SHAM_O))
OBJECTS = $(SHAM_O)
ifndef buildworld
  include ../mk/sham
  include ../mk/component
endif
