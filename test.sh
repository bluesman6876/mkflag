#!/bin/sh
#./mkflag -t "ЙЦУКЕНГШЩЗХЪФЫВАПРОЛДЖЭЯЧСМИТЬБЮЁйцукенгшщзхъфывапролджэячсмитьбюё" -f cyr -s $1 -c ff000000,b2d3d3d1,ff800000
#./mkflag -t "QWERTYUIOPASDFGHJKLZXVBNMqwertyuiopasdfghjklzxcvbnm" -f lat -s $1 -c ff000000,b2d3d3d1,ff800000
#./mkflag -t "\`1234567890-=~\!@#$%^&*()_+[]\\{}|;':\",./<>?" -f sym -s $1 -c ff000000,b2d3d3d1,ff800000
#./mkflag -t "ЙЦУКЕНГШЩЗХЪФЫВАПРОЛДЖЭЯЧСМИТЬБЮЁйцукенгшщзхъфывапролджэячсмитьбюё" -f cyr -s $1 -c ff000000,b2d3d3d1,ff800000 -l m.png
#./mkflag -t "QWERTYUIOPASDFGHJKLZXVBNMqwertyuiopasdfghjklzxcvbnm" -f lat -s $1 -c ff000000,b2d3d3d1,ff800000 -l m.png
./mkflag -t "\`1234567890-=~\!@#$%^&*()_+[]\\{}|;':\",./<>?" -f sym -s $1 -c ff000000,b2d3d3d1,ff800000 -l m.png
