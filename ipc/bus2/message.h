#ifndef __BUS1_MESSAGE_H
#define __BUS1_MESSAGE_H

/*
 * Copyright (C) 2013-2016 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 */

/**
 * DOC: Messages
 *
 * XXX
 */

#include <linux/kernel.h>
#include <linux/kref.h>
#include "util/flist.h"
#include "util/queue.h"

struct bus1_cmd_send;
struct bus1_handle;
struct bus1_peer;
struct bus1_pool_slice;
struct bus1_user;
struct cred;
struct file;
struct iovec;
struct pid;

/**
 * struct bus1_factory - message factory
 * @peer:			sending peer
 * @param:			factory parameters
 * @cred:			sender credentials
 * @pid:			sender PID
 * @tid:			sender TID
 * @on_stack:			whether object lives on stack
 * @has_secctx:			whether secctx has been set
 * @length_vecs:		total length of data in vectors
 * @n_vecs:			number of vectors
 * @n_handles:			number of handles
 * @n_files:			number of files
 * @n_secctx:			length of secctx
 * @vecs:			vector array
 * @files:			file array
 * @secctx:			allocated secctx
 * @handles:			handle array
 */
struct bus1_factory {
	struct bus1_peer *peer;
	struct bus1_cmd_send *param;
	const struct cred *cred;
	struct pid *pid;
	struct pid *tid;

	bool on_stack : 1;
	bool has_secctx : 1;

	size_t length_vecs;
	size_t n_vecs;
	size_t n_handles;
	size_t n_files;
	u32 n_secctx;
	struct iovec *vecs;
	struct file **files;
	char *secctx;

	struct bus1_flist handles[];
};

/**
 * struct bus1_message - data messages
 * @ref:			reference counter
 * @qnode:			embedded queue node
 * @flags:			message flags
 * @uid:			sender UID
 * @gid:			sender GID
 * @pid:			sender PID
 * @tid:			sender TID
 * @dst:			destination handle
 * @user:			sending user
 * @slice:			actual message data
 * @files:			passed file descriptors
 * @n_handles:			number of handles transmitted
 * @n_bytes:			number of user-bytes transmitted
 * @n_files:			number of files transmitted
 * @n_secctx:			number of bytes of security context transmitted
 * @handles:			passed handles
 */
struct bus1_message {
	struct kref ref;
	struct bus1_queue_node qnode;
	struct bus1_handle *dst;
	struct bus1_user *user;

	u64 flags;
	uid_t uid;
	gid_t gid;
	pid_t pid;
	pid_t tid;

	size_t n_bytes;
	size_t n_handles;
	size_t n_files;
	size_t n_secctx;
	struct bus1_pool_slice *slice;
	struct file **files;

	struct bus1_flist handles[];
};

struct bus1_factory *bus1_factory_new(struct bus1_peer *peer,
				      struct bus1_cmd_send *param,
				      void *stack,
				      size_t n_stack);
struct bus1_factory *bus1_factory_free(struct bus1_factory *f);
struct bus1_message *bus1_factory_instantiate(struct bus1_factory *f,
					      struct bus1_handle *handle);

void bus1_message_free(struct kref *k);

/**
 * bus1_message_ref() - acquire object reference
 * @m:			message to operate on, or NULL
 *
 * This acquires a single reference to @m. The caller must already hold a
 * reference when calling this.
 *
 * If @m is NULL, this is a no-op.
 *
 * Return: @m is returned.
 */
static inline struct bus1_message *bus1_message_ref(struct bus1_message *m)
{
	if (m)
		kref_get(&m->ref);
	return m;
}

/**
 * bus1_message_unref() - release object reference
 * @m:			message to operate on, or NULL
 *
 * This releases a single object reference to @m. If the reference counter
 * drops to 0, the message is destroyed.
 *
 * If @m is NULL, this is a no-op.
 *
 * Return: NULL is returned.
 */
static inline struct bus1_message *bus1_message_unref(struct bus1_message *m)
{
	if (m)
		kref_put(&m->ref, bus1_message_free);
	return NULL;
}

#endif /* __BUS1_MESSAGE_H */
