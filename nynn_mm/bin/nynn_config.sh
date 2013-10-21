#!/bin/bash
export blocksz=$((2**9))

export chunk_block_max=$((2**23))
export chunk_vertex_max=$((2**23))
export chunk_block_size=$blocksz

export meta_chunk_path_size=64
export meta_chunk_entry_max=128
export meta_chunk_lock_max=32
export meta_chunk_lock_slot_max=128
export meta_cache_lock_slot_max=128

export cache_block_size=$blocksz
export cache_head_max=$((2**10))
export cache_block_max=$((2**16))

