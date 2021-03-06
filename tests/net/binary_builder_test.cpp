/*******************************************************************************
 * tests/net/binary_builder_test.cpp
 *
 * Part of Project Thrill - http://project-thrill.org
 *
 * Copyright (C) 2015 Timo Bingmann <tb@panthema.net>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <thrill/common/string.hpp>
#include <thrill/net/buffer_builder.hpp>
#include <thrill/net/buffer_reader.hpp>
#include <thrill/net/fixed_buffer_builder.hpp>

#include <gtest/gtest.h>

using thrill::net::BufferBuilder;
using thrill::net::BufferRef;
using thrill::net::BufferReader;
using thrill::net::Buffer;
using thrill::common::Hexdump;

TEST(BufferBuilder, Test1) {
    // construct a binary blob
    BufferBuilder bb;
    {
        bb.Put<unsigned int>(1);
        bb.PutString("test");

        bb.PutVarint(42);
        bb.PutVarint(12345678);
    }

    // read binary block and verify content

    BufferRef bbr = BufferRef(bb);

    LOG1 << bbr.size();

    const unsigned char bb_data[] = {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        // bb.Put<unsigned int>(1)
        0x01, 0x00, 0x00, 0x00,
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        // bb.Put<unsigned int>(1)
        0x00, 0x00, 0x00, 0x01,
#endif
        // bb.PutString("test")
        0x04, 0x74, 0x65, 0x73, 0x74,
        // bb.PutVarint(42);
        0x2a,
        // bb.PutVarint(12345678);
        0xce, 0xc2, 0xf1, 0x05,
    };

    BufferRef bb_verify(bb_data, sizeof(bb_data));

    if (bbr != bb_verify)
        LOG1 << bbr.ToString();

    LOG0 << Hexdump(bbr.ToString());
    LOG0 << Hexdump(bb_verify.ToString());

    ASSERT_EQ(bbr, bb_verify);

    // read binary block using binary_reader

    BufferReader br = BufferRef(bb);

    ASSERT_EQ(br.Get<unsigned int>(), 1u);
    ASSERT_EQ(br.GetString(), "test");
    ASSERT_EQ(br.GetVarint(), 42u);
    ASSERT_EQ(br.GetVarint(), 12345678u);

    ASSERT_TRUE(br.empty());

    // MOVE origin bb (which still exists) into a net::Buffer

    ASSERT_EQ(bb.size(), sizeof(bb_data));
    Buffer nb = bb.ToBuffer();

    ASSERT_EQ(bb.size(), 0u);
    ASSERT_EQ(nb.size(), sizeof(bb_data));
}

namespace thrill {
namespace net {

template class FixedBufferBuilder<42>;

} // namespace net
} // namespace thrill

/******************************************************************************/
