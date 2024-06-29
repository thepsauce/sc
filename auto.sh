#!/bin/bash

test_file="parse"

while : ; do

    build --auto-exec -t "$test_file"

    sleep 1

done
