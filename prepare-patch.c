#include <endian.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "squashfs_fs.h"
#include "squashfs_swap.h"

int main(int argc, char **argv) {
	struct stat stat_buf[1];
	const int stat_result = fstat(STDIN_FILENO, stat_buf);
	if (stat_result == -1) {
		perror("stat");
		return 1;
	}

	if (stat_buf->st_size < SQUASHFS_START + sizeof(struct squashfs_super_block)) {
		fprintf(stderr, "image too small for super block\n");
		return 1;
	}

	void * const image = mmap(NULL, stat_buf->st_size, PROT_READ, MAP_SHARED, STDIN_FILENO, 0);
	if (image == MAP_FAILED) {
		perror("mmap");
		return 1;
	}

	struct squashfs_super_block sblk[1];
	SQUASHFS_SWAP_SUPER_BLOCK(image + SQUASHFS_START, sblk);

	if (sblk->s_magic != SQUASHFS_MAGIC) {
		fprintf(stderr, "wrong magic: 0x%08x != 0x%08x\n", sblk->s_magic, SQUASHFS_MAGIC);
		return 1;
	}

	if (sblk->s_major != SQUASHFS_MAJOR || sblk->s_minor != SQUASHFS_MINOR) {
		fprintf(stderr, "wrong version: %hu.%hu != %d.%d\n", sblk->s_major, sblk->s_minor, SQUASHFS_MAJOR, SQUASHFS_MINOR);
		return 1;
	}

	if (sblk->no_ids != 2 || SQUASHFS_ID_BLOCKS(sblk->no_ids) != 1) {
		fprintf(stderr, "wrong number of ids: %hu in %zu blocks\n", sblk->no_ids, SQUASHFS_ID_BLOCKS(sblk->no_ids));
		return 1;
	}

	if (stat_buf->st_size < sblk->id_table_start + SQUASHFS_ID_BLOCK_BYTES(sblk->no_ids)) {
		fprintf(stderr, "index list goes past EOF\n");
		return 1;
	}

	long long id_index_table[1];
	SQUASHFS_SWAP_ID_BLOCKS(image + sblk->id_table_start, id_index_table, 1);

	if (stat_buf->st_size < id_index_table[0] + sizeof(unsigned short) + SQUASHFS_ID_BYTES(sblk->no_ids)) {
		fprintf(stderr, "ID block goes past EOF\n");
		return 1;
	}

	unsigned short c_byte[1];
	SQUASHFS_SWAP_SHORTS(image + id_index_table[0], c_byte, 1);
	if (c_byte[0] != (SQUASHFS_COMPRESSED_BIT | sizeof(unsigned int [2]))) {
		fprintf(stderr, "unexpected ID block c_byte: 0x%04hx != 0x%04zx\n", c_byte[0], SQUASHFS_COMPRESSED_BIT | sizeof(unsigned int [2]));
		return 1;
	}

	unsigned int id_table[2];
	SQUASHFS_SWAP_INTS(image + id_index_table[0] + sizeof(unsigned short), id_table, 2);
	if (id_table[0] != DUMMY_UID || id_table[1] != DUMMY_GID) {
		fprintf(stderr, "unexpected id table: %u/%u != %u/%u\n", id_table[0], id_table[1], DUMMY_UID, DUMMY_GID);
		return 1;
	}

	printf("%llu\n", id_index_table[0] + sizeof(unsigned short));

	return 0;
}
