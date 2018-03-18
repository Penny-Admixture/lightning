#include "gossip_store.h"

#include <ccan/endian/endian.h>
#include <ccan/read_write_all/read_write_all.h>
#include <common/status.h>
#include <fcntl.h>

#define GOSSIP_STORE_FILENAME "gossip_store"

struct gossip_store {
	int read_fd, write_fd;
};

struct gossip_store *gossip_store_new(const tal_t *ctx)
{
	struct gossip_store *gs = tal(ctx, struct gossip_store);
	gs->write_fd = open(GOSSIP_STORE_FILENAME, O_RDWR|O_APPEND|O_CREAT, 0600);
	gs->read_fd = open(GOSSIP_STORE_FILENAME, O_RDONLY);
	return gs;
}

void gossip_store_append(struct gossip_store *gs, const u8 *msg)
{
	u16 msglen = tal_len(msg);
	beint16_t belen = cpu_to_be16(msglen);

	write_all(gs->write_fd, &belen, sizeof(belen));
	write_all(gs->write_fd, msg, msglen);
}

const u8 *gossip_store_read_next(const tal_t *ctx, struct gossip_store *gs)
{
	beint16_t belen;
	u16 msglen;
	u8 *msg;
	if (!read_all(gs->read_fd, &belen, sizeof(belen)))
		return NULL;

	msglen = be16_to_cpu(belen);
	msg = tal_arr(ctx, u8, msglen);

	if (!read_all(gs->read_fd, msg, msglen))
		status_failed(
		    STATUS_FAIL_INTERNAL_ERROR,
		    "Short read from gossip-store, expected lenght %d", msglen);

	return msg;
}