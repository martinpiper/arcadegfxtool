# arcadegfxtool
A simple tool for converting arcade roms to bmp and bmp to arcade roms

# building 
We use https://github.com/bkaradzic/GENie  
```genie gmake``` 

```make```

though 

```gcc decode_test.c decode.c bmp.c```

should work 

# commandline 
~~~
   NAMEOFDRIVER.INI should always be 1st argument.
   -i alone, imports for ALL codecs in the decoder.
   -i 1 imports for entry 1 ( the second one ) etc.
   -x alone, exports ALL codecs in the decoder.
   -x 1 exports just decoder 1.
   -q don't show image viewer for TIGR.
   -w write roms back out ( after import ) should be last.
~~~

# drivers 
.INI files or drivers are from 

http://umlautllama.com/projects/turacocl/#drivers 

TuracoCL drivers are for old rom sets so may need rom names fixing up

huge thanks to them for releasing this and doing the work 

I've only tested 4 different ones so YMMV

GhostNGoblins,Pacman,BlackTiger,and BombJack
	


