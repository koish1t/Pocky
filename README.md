# Pocky

A 3DS mod manager built on top of Checkpoint v1.0.0 written in C++.

Literally only made because I wanna be able to switch mods that I use without using ftpd to connect to my laptop every damn second.

This is still a heavy work in progress!

# How to use it

When you first open the app it makes folders for all of your games.

Then you gotta make the mods folder for it and then add the romfs folder to it.

```
/3ds/Pocky/mods/TitleId GameName/
    ├── ModName/
        └── romfs/
```

Then you can apply or disable your mod!

## Working path

Pocky uses the following folders to store the files it generates. Note that all the working directories are automatically generated on first launch (or when Pocky finds a new title that doesn't have a working directory yet).

* **`sdmc:/3ds/Pocky`**: root path
* **`sdmc:/3ds/Pocky/mods/<game title>`**: root path for all the mods for a generic game

## License

This project is licensed under the GNU GPLv3. See [LICENSE.md](https://github.com/YoPhlox/Pocky/blob/master/LICENSE) for details.

# Tested Games

- Hatsune Miku: Project Mirai DX

# Credits

* YoPhlox - Pocky
* BernardoGiordano - Checkpoint
