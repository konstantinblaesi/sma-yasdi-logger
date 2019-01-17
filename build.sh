#!/bin/sh
set -e

FLATBUFFER_IDL="flatbuffers/*.fbs"
GENERATED_SOURCES_DIR="flatbuffers/include"

rm -rf "$GENERATED_SOURCES_DIR"
mkdir "$GENERATED_SOURCES_DIR"

for f in $FLATBUFFER_IDL; do
    flatc --cpp -o "$GENERATED_SOURCES_DIR" "$f"
done