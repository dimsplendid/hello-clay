# Hello Clay

Test project for learning Clay 🤓.

> https://github.com/nicbarker/clay

## Start Up

Using `nob.h` created by tsoding.
> https://github.com/tsoding/nob.h
> Just test on Windows with msys64 now, the nob.c need modify environments in different PC.

```bash
$ clang nob.c -o nob.exe # kick off, only need once
$ ./nob.exe              # compile
$ ./bin/hello-clay.exe
```
## Modify

- Add unicode support at `libs/clay_renderer_raylib.c`, change the measure text function to `Raylib_MeasureTextUnicode`.

```c
int main(void) {
    ...
    Clay_SetMeasureTextFunction(Raylib_MeasureTextUnicode, fonts);
    ...
}
```
- Add CJK words wrap function to `libs/clay.h`, simply make a Kanji a word :D, not sure is this would cause performance impact.
  ![](demo/demo-1.png)
  > I watch the task manager and it is increasing the usage when showing kanji.

## TODO

- There's bugs in CJK wrap, sometimes it would show `??`. Though I check the out is unicode cutting issue, I still not found where's the problems.
  ![](demo/demo-bug.png)
- Try to reduce the performance impact by CJK.
- Add simple game and interact with UI.
