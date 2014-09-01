#!/bin/sh
./mkflag -t "ЙЦУКЕНГШЩЗХЪФЫВАПРОЛДЖЭЯЧСМИТЬБЮЁйцукенгшщзхъфывапролджэячсмитьбюё" -f cyr -s $2 -c 000000,d3d3d1,800000
./mkflag -t "QWERTYUIOPASDFGHJKLZXVBNMqwertyuiopasdfghjklzxcvbnm" -f lat -s $2 -c 000000,d3d3d1,800000
./mkflag -t "\`1234567890-=~!@#$%^&*()_+[]\\{}|;':\",./<>?" -f sym -s $2 -c 000000,d3d3d1,800000
