/*
 * Copyright (c) 2002-2017 "Neo Technology,"
 * Network Engine for Objects in Lund AB [http://neotechnology.com]
 *
 * This file is part of Neo4j.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <math.h>
#include "bolt.h"
#include "test_values.h"


void _dump(struct BoltValue* value)
{
    BoltValue_write(stdout, value);
    fprintf(stdout, "\n");
}

void test_null()
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_Null(value);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_NULL);
    BoltValue_destroy(value);
}

void _test_list()
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_List(value, 6);
    BoltValue_to_Int32(BoltList_value(value, 0), 1234567);
    BoltValue_to_Int32(BoltList_value(value, 1), 2345678);
    BoltValue_to_Int32(BoltList_value(value, 2), 3456789);
    BoltValue_to_UTF8(BoltList_value(value, 3), "hello", 5);
    BoltValue_to_List(BoltList_value(value, 5), 3);
    BoltValue_to_Num8(BoltList_value(BoltList_value(value, 5), 0), 77);
    BoltValue_to_Num8(BoltList_value(BoltList_value(value, 5), 1), 88);
    BoltValue_to_Byte(BoltList_value(BoltList_value(value, 5), 2), 99);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_LIST);
    assert(value->size == 6);
    BoltValue_destroy(value);
}

void _test_empty_list()
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_List(value, 0);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_LIST);
    assert(value->size == 0);
    BoltValue_destroy(value);
}

void _test_list_growth()
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_List(value, 0);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_LIST);
    assert(value->size == 0);
    for (int i = 0; i < 3; i++)
    {
        int size = i + 1;
        BoltList_resize(value, size);
        BoltValue_to_Int8(BoltList_value(value, i), (int8_t)(size));
        _dump(value);
        assert(BoltValue_type(value) == BOLT_LIST);
        assert(value->size == size);
    }
    BoltValue_destroy(value);
}

void _test_list_shrinkage()
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_List(value, 3);
    BoltValue_to_Int8(BoltList_value(value, 0), 1);
    BoltValue_to_Int8(BoltList_value(value, 1), 2);
    BoltValue_to_Int8(BoltList_value(value, 2), 3);
    assert(BoltValue_type(value) == BOLT_LIST);
    assert(value->size == 3);
    for (int size = 3; size >= 0; size--)
    {
        BoltList_resize(value, size);
        _dump(value);
        assert(BoltValue_type(value) == BOLT_LIST);
        assert(value->size == size);
    }
    BoltValue_destroy(value);
}

void test_list()
{
    _test_list();
    _test_empty_list();
    _test_list_growth();
    _test_list_shrinkage();
}

void _test_utf8_dictionary()
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_UTF8Dictionary(value, 4);
    BoltValue_to_Int8(BoltUTF8Dictionary_with_key(value, 0, "a", 1), 1);
    BoltValue_to_Int8(BoltUTF8Dictionary_with_key(value, 1, "b", 1), 2);
    BoltValue_to_Int8(BoltUTF8Dictionary_with_key(value, 2, "c", 1), 3);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_UTF8_DICTIONARY);
    assert(value->size == 4);
    BoltValue_destroy(value);
}

void _test_empty_utf8_dictionary()
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_UTF8Dictionary(value, 0);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_UTF8_DICTIONARY);
    assert(value->size == 0);
    BoltValue_destroy(value);
}

void _test_single_entry_utf8_dictionary()
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_UTF8Dictionary(value, 1);
    BoltValue_to_UTF8(BoltUTF8Dictionary_with_key(value, 0, "hello", 5), "world", 5);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_UTF8_DICTIONARY);
    assert(value->size == 1);
    BoltValue_destroy(value);
}

void _test_utf8_dictionary_growth()
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_UTF8Dictionary(value, 0);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_UTF8_DICTIONARY);
    assert(value->size == 0);
    for (int i = 0; i < 3; i++)
    {
        int size = i + 1;
        BoltUTF8Dictionary_resize(value, size);
        BoltValue_to_Int8(BoltUTF8Dictionary_with_key(value, i, "key", 3), (int8_t)(size));
        _dump(value);
        assert(BoltValue_type(value) == BOLT_UTF8_DICTIONARY);
        assert(value->size == size);
    }
    BoltValue_destroy(value);
}

void _test_utf8_dictionary_shrinkage()
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_UTF8Dictionary(value, 3);
    BoltValue_to_Int8(BoltUTF8Dictionary_with_key(value, 0, "a", 1), 1);
    BoltValue_to_Int8(BoltUTF8Dictionary_with_key(value, 1, "b", 1), 2);
    BoltValue_to_Int8(BoltUTF8Dictionary_with_key(value, 2, "c", 1), 3);
    assert(BoltValue_type(value) == BOLT_UTF8_DICTIONARY);
    assert(value->size == 3);
    for (int size = 3; size >= 0; size--)
    {
        BoltUTF8Dictionary_resize(value, size);
        _dump(value);
        assert(BoltValue_type(value) == BOLT_UTF8_DICTIONARY);
        assert(value->size == size);
    }
    BoltValue_destroy(value);
}

void test_utf8_dictionary()
{
    _test_utf8_dictionary();
    _test_empty_utf8_dictionary();
    _test_single_entry_utf8_dictionary();
    _test_utf8_dictionary_growth();
    _test_utf8_dictionary_shrinkage();
}

void test_bit()
{
    struct BoltValue* value = BoltValue_create();
    for (char i = 0; i <= 1; i++)
    {
        BoltValue_to_Bit(value, i);
        _dump(value);
        assert(BoltValue_type(value) == BOLT_BIT);
        assert(BoltBit_get(value) == i);
    }
    BoltValue_destroy(value);
}

void test_bit_array()
{
    struct BoltValue* value = BoltValue_create();
    int32_t size = 2;
    char array[] = {0, 1};
    BoltValue_to_BitArray(value, array, size);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_BIT_ARRAY);
    for (int i = 0; i < size; i++)
    {
        assert(BoltBitArray_get(value, i) == array[i]);
    }
    BoltValue_destroy(value);
}

void test_byte()
{
    struct BoltValue* value = BoltValue_create();
    for (int i = 0x00; i <= 0xFF; i++)
    {
        BoltValue_to_Byte(value, (char)(i));
        _dump(value);
        assert(BoltValue_type(value) == BOLT_BYTE);
        assert(BoltByte_get(value) == (char)(i));
    }
    BoltValue_destroy(value);
}

void test_byte_array()
{
    struct BoltValue* value = BoltValue_create();
    char array[] = ("\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\r\x0e\x0f"
            "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
            " !\"#$%&'()*+,-./0123456789:;<=>?"
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
            "`abcdefghijklmnopqrstuvwxyz{|}~\x7f"
            "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
            "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
            "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf"
            "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
            "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf"
            "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
            "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef"
            "\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff");
    int32_t size = sizeof(array) - 1;
    BoltValue_to_ByteArray(value, array, size);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_BYTE_ARRAY);
    for (int i = 0; i < size; i++)
    {
        assert(BoltByteArray_get(value, i) == array[i]);
    }
    BoltValue_destroy(value);
}

void _test_utf8(char* text, int32_t text_size)
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_UTF8(value, text, text_size);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_UTF8);
    assert(value->size == text_size);
    const char* stored_text = BoltUTF8_get(value);
    assert(strncmp(text, stored_text, (size_t)(text_size)) == 0);
    BoltValue_destroy(value);
}

void test_utf8()
{
    _test_utf8("", 0);
    _test_utf8("hello, world", 12);
    _test_utf8("there is a null character -> \x00 <- in the middle of this string", 62);
    _test_utf8("back to a short one", 19);
}

void test_utf8_array()
{
    struct BoltValue* value = BoltValue_create();
    char* text;
    int32_t size;
    BoltValue_to_UTF8Array(value, 5);
    BoltUTF8Array_put(value, 0, "hello", 5);
    BoltUTF8Array_put(value, 1, "world", 5);
    BoltUTF8Array_put(value, 2, "here is a very very very very very very very very long string", 61);
    BoltUTF8Array_put(value, 3, "", 0);
    BoltUTF8Array_put(value, 4, "that last one was empty!!", 25);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_UTF8_ARRAY);
    assert(value->size == 5);

    text = BoltUTF8Array_get(value, 0);
    size = BoltUTF8Array_get_size(value, 0);
    assert(strncmp(text, "hello", (size_t)(size)) == 0);

    text = BoltUTF8Array_get(value, 1);
    size = BoltUTF8Array_get_size(value, 1);
    assert(strncmp(text, "world", (size_t)(size)) == 0);

    text = BoltUTF8Array_get(value, 2);
    size = BoltUTF8Array_get_size(value, 2);
    assert(strncmp(text, "here is a very very very very very very very very long string", (size_t)(size)) == 0);

    text = BoltUTF8Array_get(value, 3);
    size = BoltUTF8Array_get_size(value, 3);
    assert(strncmp(text, "", (size_t)(size)) == 0);

    text = BoltUTF8Array_get(value, 4);
    size = BoltUTF8Array_get_size(value, 4);
    assert(strncmp(text, "that last one was empty!!", (size_t)(size)) == 0);

    BoltValue_destroy(value);
}

int test_num8()
{
    struct BoltValue* value = BoltValue_create();
    int n = 0;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x100)
    {
        BoltValue_to_Num8(value, (uint8_t)(x));
        _dump(value);
        assert(BoltValue_type(value) == BOLT_NUM8);
        assert(BoltNum8_get(value) == x);
        n += 1, z = x + y, x = y, y = z;
    }
    BoltValue_destroy(value);
    return n;
}

void test_num8_array(int size)
{
    struct BoltValue* value = BoltValue_create();
    uint8_t array[size];
    int n = 0;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x100)
    {
        array[n] = (uint8_t)(x);
        n += 1, z = x + y, x = y, y = z;
    }
    BoltValue_to_Num8Array(value, array, size);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_NUM8_ARRAY);
    for (int i = 0; i < size; i++)
    {
        assert(BoltNum8Array_get(value, i) == array[i]);
    }
    BoltValue_destroy(value);
}

int test_num16()
{
    struct BoltValue* value = BoltValue_create();
    int n = 0;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x10000)
    {
        BoltValue_to_Num16(value, (uint16_t)(x));
        _dump(value);
        assert(BoltValue_type(value) == BOLT_NUM16);
        assert(BoltNum16_get(value) == x);
        n += 1, z = x + y, x = y, y = z;
    }
    BoltValue_destroy(value);
    return n;
}

void test_num16_array(int size)
{
    struct BoltValue* value = BoltValue_create();
    uint16_t array[size];
    int n = 0;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x10000)
    {
        array[n] = (uint16_t)(x);
        n += 1, z = x + y, x = y, y = z;
    }
    BoltValue_to_Num16Array(value, array, size);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_NUM16_ARRAY);
    for (int i = 0; i < size; i++)
    {
        assert(BoltNum16Array_get(value, i) == array[i]);
    }
    BoltValue_destroy(value);
}

int test_num32()
{
    struct BoltValue* value = BoltValue_create();
    int n = 0;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x100000000)
    {
        BoltValue_to_Num32(value, (uint32_t)(x));
        _dump(value);
        assert(BoltValue_type(value) == BOLT_NUM32);
        assert(BoltNum32_get(value) == x);
        n += 1, z = x + y, x = y, y = z;
    }
    BoltValue_destroy(value);
    return n;
}

void test_num32_array(int size)
{
    struct BoltValue* value = BoltValue_create();
    uint32_t array[size];
    int n = 0;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x100000000)
    {
        array[n] = (uint32_t)(x);
        n += 1, z = x + y, x = y, y = z;
    }
    BoltValue_to_Num32Array(value, array, size);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_NUM32_ARRAY);
    for (int i = 0; i < size; i++)
    {
        assert(BoltNum32Array_get(value, i) == array[i]);
    }
    BoltValue_destroy(value);
}

int test_num64()
{
    struct BoltValue* value = BoltValue_create();
    int n = 0;
    unsigned long long x = 0, y = 1, z;
    while (x < 0xA000000000000000L)
    {
        BoltValue_to_Num64(value, (uint64_t)(x));
        _dump(value);
        assert(BoltValue_type(value) == BOLT_NUM64);
        assert(BoltNum64_get(value) == x);
        n += 1, z = x + y, x = y, y = z;
    }
    BoltValue_destroy(value);
    return n;
}

void test_num64_array(int size)
{
    struct BoltValue* value = BoltValue_create();
    uint64_t array[size];
    int n = 0;
    unsigned long long x = 0, y = 1, z;
    while (x < 0xA000000000000000L)
    {
        array[n] = (uint64_t)(x);
        n += 1, z = x + y, x = y, y = z;
    }
    BoltValue_to_Num64Array(value, array, size);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_NUM64_ARRAY);
    for (int i = 0; i < size; i++)
    {
        assert(BoltNum64Array_get(value, i) == array[i]);
    }
    BoltValue_destroy(value);
}

int test_int8()
{
    struct BoltValue* value = BoltValue_create();
    int n = 0, s = 1;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x80)
    {
        BoltValue_to_Int8(value, (int8_t)(s * x));
        _dump(value);
        assert(BoltValue_type(value) == BOLT_INT8);
        assert(BoltInt8_get(value) == s * x);
        n += 1, s = -s, z = x + y, x = y, y = z;
    }
    BoltValue_destroy(value);
    return n;
}

void test_int8_array(int size)
{
    struct BoltValue* value = BoltValue_create();
    int8_t array[size];
    int n = 0, s = 1;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x80)
    {
        array[n] = (int8_t)(s * x);
        n += 1, s = -s, z = x + y, x = y, y = z;
    }
    BoltValue_to_Int8Array(value, array, size);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_INT8_ARRAY);
    for (int i = 0; i < size; i++)
    {
        assert(BoltInt8Array_get(value, i) == array[i]);
    }
    BoltValue_destroy(value);
}

int test_int16()
{
    struct BoltValue* value = BoltValue_create();
    int n = 0, s = 1;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x8000)
    {
        BoltValue_to_Int16(value, (int16_t)(s * x));
        _dump(value);
        assert(BoltValue_type(value) == BOLT_INT16);
        assert(BoltInt16_get(value) == s * x);
        n += 1, s = -s, z = x + y, x = y, y = z;
    }
    BoltValue_destroy(value);
    return n;
}

void test_int16_array(int size)
{
    struct BoltValue* value = BoltValue_create();
    int16_t array[size];
    int n = 0, s = 1;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x8000)
    {
        array[n] = (int16_t)(s * x);
        n += 1, s = -s, z = x + y, x = y, y = z;
    }
    BoltValue_to_Int16Array(value, array, size);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_INT16_ARRAY);
    for (int i = 0; i < size; i++)
    {
        assert(BoltInt16Array_get(value, i) == array[i]);
    }
    BoltValue_destroy(value);
}

int test_int32()
{
    struct BoltValue* value = BoltValue_create();
    int n = 0, s = 1;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x80000000)
    {
        BoltValue_to_Int32(value, (int32_t)(s * x));
        _dump(value);
        assert(BoltValue_type(value) == BOLT_INT32);
        assert(BoltInt32_get(value) == s * x);
        n += 1, s = -s, z = x + y, x = y, y = z;
    }
    BoltValue_destroy(value);
    return n;
}

void test_int32_array(int size)
{
    struct BoltValue* value = BoltValue_create();
    int32_t array[size];
    int n = 0, s = 1;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x80000000)
    {
        array[n] = (int32_t)(s * x);
        n += 1, s = -s, z = x + y, x = y, y = z;
    }
    BoltValue_to_Int32Array(value, array, size);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_INT32_ARRAY);
    for (int i = 0; i < size; i++)
    {
        assert(BoltInt32Array_get(value, i) == array[i]);
    }
    BoltValue_destroy(value);
}

int test_int64()
{
    struct BoltValue* value = BoltValue_create();
    int n = 0, s = 1;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x8000000000000000)
    {
        BoltValue_to_Int64(value, (int64_t)(s * x));
        _dump(value);
        assert(BoltValue_type(value) == BOLT_INT64);
        assert(BoltInt64_get(value) == s * x);
        n += 1, s = -s, z = x + y, x = y, y = z;
    }
    BoltValue_destroy(value);
    return n;
}

void test_int64_array(int size)
{
    struct BoltValue* value = BoltValue_create();
    int64_t array[size];
    int n = 0, s = 1;
    unsigned long long x = 0, y = 1, z;
    while (x < 0x8000000000000000)
    {
        array[n] = (int64_t)(s * x);
        n += 1, s = -s, z = x + y, x = y, y = z;
    }
    BoltValue_to_Int64Array(value, array, size);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_INT64_ARRAY);
    for (int i = 0; i < size; i++)
    {
        assert(BoltInt64Array_get(value, i) == array[i]);
    }
    BoltValue_destroy(value);
}

void _test_float32(float x)
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_Float32(value, x);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_FLOAT32);
    assert(BoltFloat32_get(value) == x ||
           (isnanf(BoltFloat32_get(value)) && isnanf(x)));
    BoltValue_destroy(value);
}

void test_float32()
{
    _test_float32(0.0F);
    _test_float32(0.375F);
    _test_float32(1.0F);
    _test_float32(-1.0F);
    _test_float32(3.14159F);
    _test_float32(-3.14159F);
    _test_float32(6.02214086e23F);
    _test_float32(3.402823e38F);
    _test_float32(INFINITY);
    _test_float32(-INFINITY);
    _test_float32(NAN);
}

void test_float32_array()
{
    struct BoltValue* value = BoltValue_create();
    float array[11] = {0.0F, 0.375F, 1.0F, -1.0F, 3.14159F, -3.14159F,
                       6.02214086e23F, 3.402823e38F, INFINITY, -INFINITY, NAN};
    BoltValue_to_Float32Array(value, array, 11);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_FLOAT32_ARRAY);
    assert(value->size == 11);
    assert(BoltFloat32Array_get(value, 0) == 0.0F);
    assert(BoltFloat32Array_get(value, 1) == 0.375F);
    assert(BoltFloat32Array_get(value, 2) == 1.0F);
    assert(BoltFloat32Array_get(value, 3) == -1.0F);
    assert(BoltFloat32Array_get(value, 4) == 3.14159F);
    assert(BoltFloat32Array_get(value, 5) == -3.14159F);
    assert(BoltFloat32Array_get(value, 6) == 6.02214086e23F);
    assert(BoltFloat32Array_get(value, 7) == 3.402823e38F);
    assert(BoltFloat32Array_get(value, 8) == INFINITY);
    assert(BoltFloat32Array_get(value, 9) == -INFINITY);
    assert(isnanf(BoltFloat32Array_get(value, 10)));
    BoltValue_destroy(value);
}

void _test_float64(double x)
{
    struct BoltValue* value = BoltValue_create();
    BoltValue_to_Float64(value, x);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_FLOAT64);
    assert(BoltFloat64_get(value) == x ||
           (isnan(BoltFloat64_get(value)) && isnan(x)));
    BoltValue_destroy(value);
}

void test_float64()
{
    _test_float64(0.0);
    _test_float64(0.375);
    _test_float64(1.0);
    _test_float64(-1.0);
    _test_float64(3.14159);
    _test_float64(-3.14159);
    _test_float64(6.02214086e23);
    _test_float64(3.402823e38);
    _test_float64(INFINITY);
    _test_float64(-INFINITY);
    _test_float64(NAN);
}

void test_structure()
{
    struct BoltValue* value = BoltValue_create();
    const int NODE = 0xA0;
    BoltValue_to_Structure(value, NODE, 3);
    struct BoltValue* id = BoltStructure_value(value, 0);
    struct BoltValue* labels = BoltStructure_value(value, 1);
    struct BoltValue* properties = BoltStructure_value(value, 2);
    BoltValue_to_Int64(id, 123);
    BoltValue_to_UTF8Array(labels, 2);
    BoltUTF8Array_put(labels, 0, "Person", 6);
    BoltUTF8Array_put(labels, 1, "Employee", 8);
    BoltValue_to_UTF8Dictionary(properties, 2);
    BoltValue_to_UTF8(BoltUTF8Dictionary_with_key(properties, 0, "name", 4), "Alice", 5);
    BoltValue_to_Num8(BoltUTF8Dictionary_with_key(properties, 1, "age", 3), 33);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_STRUCTURE && BoltStructure_code(value) == NODE);
    assert(value->size == 3);
    BoltValue_destroy(value);
}

void test_structure_array()
{
    struct BoltValue* value = BoltValue_create();
    const int NODE = 0xA0;
    BoltValue_to_StructureArray(value, NODE, 2);
    for (int i = 0; i < 2; i++)
    {
        BoltStructureArray_set_size(value, i, 3);
        struct BoltValue* id = BoltStructureArray_at(value, i, 0);
        struct BoltValue* labels = BoltStructureArray_at(value, i, 1);
        struct BoltValue* properties = BoltStructureArray_at(value, i, 2);
        BoltValue_to_Int64(id, 123 + 1);
        BoltValue_to_UTF8Array(labels, 2);
        BoltUTF8Array_put(labels, 0, "Person", 6);
        BoltUTF8Array_put(labels, 1, "Employee", 8);
        BoltValue_to_UTF8Dictionary(properties, 2);
        BoltValue_to_UTF8(BoltUTF8Dictionary_with_key(properties, 0, "name", 4),
                          i == 0 ? "Alice" : "Bob", i == 0 ? 5 : 3);
        BoltValue_to_Num8(BoltUTF8Dictionary_with_key(properties, 1, "age", 3), (uint8_t)(i == 0 ? 33 : 44));
    }
    _dump(value);
    assert(BoltValue_type(value) == BOLT_STRUCTURE_ARRAY && BoltStructure_code(value) == NODE);
    assert(value->size == 2);
    BoltValue_destroy(value);
}

void test_request()
{
    struct BoltValue* value = BoltValue_create();
    const int RUN = 0xA0;
    BoltValue_to_Request(value, RUN, 2);
    struct BoltValue* statement = BoltRequest_value(value, 0);
    struct BoltValue* parameters = BoltRequest_value(value, 1);
    BoltValue_to_UTF8(statement, "RETURN $x", 9);
    BoltValue_to_UTF8Dictionary(parameters, 1);
    BoltValue_to_Int64(BoltUTF8Dictionary_with_key(parameters, 0, "x", 1), 1);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_REQUEST && BoltRequest_code(value) == RUN);
    assert(value->size == 2);
    BoltValue_destroy(value);
}

void test_summary()
{
    struct BoltValue* value = BoltValue_create();
    const int SUCCESS = 0xA0;
    BoltValue_to_Summary(value, SUCCESS, 1);
    struct BoltValue* metadata = BoltSummary_value(value, 0);
    BoltValue_to_UTF8Dictionary(metadata, 2);
    BoltValue_to_Int64(BoltUTF8Dictionary_with_key(metadata, 0, "results", 7), 100);
    BoltValue_to_Int64(BoltUTF8Dictionary_with_key(metadata, 1, "time", 4), 123456789);
    _dump(value);
    assert(BoltValue_type(value) == BOLT_SUMMARY && BoltSummary_code(value) == SUCCESS);
    assert(value->size == 1);
    BoltValue_destroy(value);
}

int test_types()
{
    test_null();
    test_list();
    test_bit();
    test_bit_array();
    test_byte();
    test_byte_array();
    test_utf8();
    test_utf8_array();
    test_utf8_dictionary();
    test_num8_array(test_num8());
    test_num16_array(test_num16());
    test_num32_array(test_num32());
    test_num64_array(test_num64());
    test_int8_array(test_int8());
    test_int16_array(test_int16());
    test_int32_array(test_int32());
    test_int64_array(test_int64());
    test_float32();
    test_float32_array();
    test_float64();
    // TODO: test_float64_array();
    test_structure();
    test_structure_array();
    test_request();
    test_summary();
    assert(BoltMem_allocated() == 0);
    printf("*******\nMemory activity: %lld\n*******\n", BoltMem_activity());
}