#/usr/bin/bash

OUT_NAME=testLua

echo "Testing Lua 5.1"
rm -f $OUT_NAME
gcc main.c -o $OUT_NAME -Ilua51/dist/include lua51/dist/x86_64-linux-gnu/lib/liblua5.1.a -lm
./$OUT_NAME

echo "Testing Lua 5.2"
rm $OUT_NAME
gcc main.c -o $OUT_NAME -Ilua52/dist/include lua52/dist/x86_64-linux-gnu/lib/liblua5.2.a -lm
./$OUT_NAME

echo "Testing Lua 5.3"
rm $OUT_NAME
$(gcc main.c -o $OUT_NAME -Ilua53/dist/include lua53/dist/x86_64-linux-gnu/lib/liblua5.3.a -lm)
./$OUT_NAME

echo "Testing Lua 5.4"
rm $OUT_NAME
gcc main.c -o $OUT_NAME -Ilua54/dist/include lua54/dist/x86_64-linux-gnu/lib/liblua5.4.a -lm
./$OUT_NAME

echo "Testing Lua JIT"
rm $OUT_NAME
gcc main.c -o $OUT_NAME -Iluajit/dist/include luajit/dist/x86_64-linux-gnu/lib/libluajit.a -lm
./$OUT_NAME

rm $OUT_NAME
