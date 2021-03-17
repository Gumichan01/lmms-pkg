# LMMS Packager #

## What is this? ##

LMMS Packager is a command-line tool that packages your LMMS project, your samples and your soundfont (SF2) files into one compressed file.


## Why? ##

As a music producer, it can be tricky to share an LMMS project that uses some external materials (samples and soundfont files) with the community,
because you have to make sure that the user has everything he/she needs to load the project properly. You may also want to have a backup package.

As an LMMS user / music producer, it is painful to import every samples manually into LMMS.
This cannot be a problem with just one sample (or soundfont file) but with more than 20 or more, it is a waste of time.

That is why LMMS Packager exists.


## What does it do? ##

It takes your project, retrieves the samples and the soundfont files used by the project and packaqes them in a *.mmpk* file.
It can also extract the package, and configure the project file.


## Show me how to use it! ##

This is how you export it.

```
$ lmms-pkg --export my-project.mmp package-directory/	# It also works with *.mmpz* files
```

This is how you import your project.

```
$ lmms-pkg --import my-package.mmpk import-directory/
```


See the [wiki](https://github.com/Gumichan01/lmms-pkg/wiki/Manual) to get more examples.


## Run it ##

You can get binaries [here](https://github.com/Gumichan01/lmms-pkg/releases).


## Build ##

### Windows ###

If you are using CodeBlocks, a project file is provided.

If you are using Visual Studio, good luck!


### Linux ###

Just do this:

```
make	# ( -.-)
``` 

You will have an `lmms-pkg` file.


## License ##

This program is under GPL v3.
