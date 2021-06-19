#! /usr/bin/env/ python3

with open("data.txt", "wb") as out:
    out.seek(( 1024 * 1024) - 1)
    out.write('\0')
