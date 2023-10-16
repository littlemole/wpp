#!/bin/bash

for (( i=0; i<$1; i++))
do

curl -k 'https://localhost:8080/css/style.css' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0' -H 'Accept: text/css,*/*;q=0.1' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Connection: keep-alive' -H 'Referer: https://localhost:8080/login' 

done

