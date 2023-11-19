1) Generally the first thing i check is how the game is running on the console and if is possible to play it, so the menu navigations and gameplay should be working fine with the gamepad at least, if not (the game uses only keyboard and mouse, for example), the rework must be made where necessary.
User handling system.
The Save is another important system to port, if it already works in the source platform, it should work in the target platform too (by serializing and save it using the console SDK).
Depending on the platform, some optimizations can be necessary to avoid memory leaks (reduce texture quality, force lower mipmap, force some audios to play as stream, etc).
I must say that the time to get the game running and playing is from two weeks up to one month, and up to two months to a complete porting (considering other platform's specific stuffs like achievements, activities, gamepad disconnection handle, etc).

2) The save system is one of the most difficult to handle, as if the save format changes often, it can broke the game and can be dificult to test