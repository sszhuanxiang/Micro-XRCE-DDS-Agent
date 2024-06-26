// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file fileInfo.c
 * This source file contains the definition of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#include "fileInfo.h"

#include <ucdr/microcdr.h>
#include <string.h>

bool fileInfo_serialize_topic(ucdrBuffer* writer, const fileInfo* topic)
{
    bool success = true;

        success &= ucdr_serialize_string(writer, topic->filename);

        success &= ucdr_serialize_uint64_t(writer, topic->timestamp);

    return success && !writer->error;
}

bool fileInfo_deserialize_topic(ucdrBuffer* reader, fileInfo* topic)
{
    bool success = true;

        success &= ucdr_deserialize_string(reader, topic->filename, 255);

        success &= ucdr_deserialize_uint64_t(reader, &topic->timestamp);

    return success && !reader->error;
}

uint32_t fileInfo_size_of_topic(const fileInfo* topic, uint32_t size)
{
    uint32_t previousSize = size;
        size += ucdr_alignment(size, 4) + 4 + (uint32_t)strlen(topic->filename) + 1;

        size += ucdr_alignment(size, 8) + 8;

    return size - previousSize;
}
