#!/bin/ksh

outfn()
	{
	print -n $1 | tr , _ | tr : _
	}

mkdir -p tmp

# Output formats

test_output()
	{
	fmt24s="jpg bmp pcx tif tif,lzw tga tga,ydown tga,16 tga,32 lbm lbm,ham6 vid img"
	for fmt24 in $fmt24s ; do
		gbmref rainbow_fish.tif tmp/24_`outfn $fmt24`.$fmt24
		xgbmv tmp/24_`outfn $fmt24`.${fmt24%%,*}
	done

	fmt8s="bmp pcx tif tif,lzw gif gif,ilace pcx tga tga,ydown lbm pgm kps iax iax,r iax,g iax,b spr img img,pal"
	for fmt8 in $fmt8s ; do
		gbmbpp -m 7x8x4 rainbow_fish.tif tmp/8_`outfn $fmt8`.$fmt8
		xgbmv tmp/8_`outfn $fmt8`.${fmt8%%,*}
	done

	fmt4s="bmp pcx tif tif,lzw gif gif,ilace pcx lbm spr img img,grey img,pal"
	for fmt4 in $fmt4s ; do
		gbmbpp -m vga rainbow_fish.tif tmp/4_`outfn $fmt4`.$fmt4
		xgbmv tmp/4_`outfn $fmt4`.${fmt4%%,*}
	done

	fmt1s="bmp pcx tif tif,lzw gif gif,ilace pcx lbm xbm spr pseg img"
	for fmt1 in $fmt1s ; do
		gbmbpp -m bw rainbow_fish.tif tmp/1_`outfn $fmt1`.$fmt1
		xgbmv tmp/1_`outfn $fmt1`.${fmt1%%,*}
	done
	}

# Mappings

test_mappings()
	{
	for map in \
	6:6:6 4:4:4 2:2:2 \
	mcut256 mcut128 mcut64 mcut32 \
	freq6:6:6:256 freq6:6:6:128 freq6:6:6:64 freq6:6:6:32 \
	tripel 8g 4x4x4 6x6x6 7x8x4 \
	4g 8 vga bw \
	; do
		gbmbpp -m $map rainbow_fish.tif tmp/map_`outfn $map`.tif,lzw
		xgbmv tmp/map_`outfn $map`.tif
	done
	}

test_output
test_mappings

print -n "Enter to remove temporary files..."
read
rm -rf tmp
