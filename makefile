static: clean
	$(MAKE) -C freetype -f my_make static
	$(MAKE) -C libpng -f my_make static
	$(MAKE) -C zlib -f my_make static
	$(MAKE) -C app -f my_make static

dynamic:clean
	$(MAKE) -C freetype -f my_make dynamic
	$(MAKE) -C libpng -f my_make dynamic
	$(MAKE) -C app -f my_make dynamic

blob: 	clean
	$(MAKE) -C freetype -f my_make blob
	$(MAKE) -C libpng -f my_make blob
	$(MAKE) -C zlib -f my_make blob
	$(MAKE) -C app -f my_make blob

clean:
	$(MAKE) -C freetype -f my_make clean
	$(MAKE) -C libpng -f my_make clean
	$(MAKE) -C zlib -f my_make clean
	$(MAKE) -C app -f my_make clean
