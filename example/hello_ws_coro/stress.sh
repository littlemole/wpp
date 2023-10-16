#!/bin/bash

rm cookies.jar
touch cookies.jar

curl -kv -c cookies.jar 'https://localhost:8080/register' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0' -H 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Content-Type: application/x-www-form-urlencoded' -H 'Connection: keep-alive' -H 'Referer: https://localhost:8080/register' -H 'Upgrade-Insecure-Requests: 1' --data 'username=test&login=test%40knitbits.de&pwd=test'

for (( i=0; i<$1; i++))
do
curl -kv -b cookies.jar -c cookies.jar 'https://localhost:8080/login' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0' -H 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Content-Type: application/x-www-form-urlencoded' -H 'Connection: keep-alive' -H 'Referer: https://localhost:8080/login' -H 'Upgrade-Insecure-Requests: 1' --data 'login=test%40knitbits.de&pwd=test'
curl -kv -b cookies.jar 'https://localhost:8080/' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0' -H 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Connection: keep-alive' -H 'Referer: https://localhost:8080/register' -H 'Upgrade-Insecure-Requests: 1'
curl -kv -b cookies.jar 'https://localhost:8080/css/style.css' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0' -H 'Accept: text/css,*/*;q=0.1' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Connection: keep-alive' -H 'Referer: https://localhost:8080/login' 
curl -kv -b cookies.jar -c cookies.jar 'https://localhost:8080/logout' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0' -H 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Connection: keep-alive' -H 'Referer: https://localhost:8080/login' -H 'Upgrade-Insecure-Requests: 1'

curl -kv -b cookies.jar 'https://localhost:8080/' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0' -H 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Connection: keep-alive' -H 'Referer: https://localhost:8080/login'  -H 'Upgrade-Insecure-Requests: 1'

#curl -kv -b cookies.jar 'https://localhost:8080/css/style.css' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0' -H 'Accept: text/css,*/*;q=0.1' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Connection: keep-alive' -H 'Referer: https://localhost:8080/login' 
#curl -kv -b cookies.jar 'https://localhost:8080/js/jquery.min.js' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0' -H 'Accept: text/css,*/*;q=0.1' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Connection: keep-alive' -H 'Referer: https://localhost:8080/login' 
#curl -kv -b cookies.jar 'https://localhost:8080/js/jquery.cookie.js' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0' -H 'Accept: text/css,*/*;q=0.1' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Connection: keep-alive' -H 'Referer: https://localhost:8080/login' 
#curl -kv -b cookies.jar 'https://localhost:8080/js/mvc.js' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0' -H 'Accept: text/css,*/*;q=0.1' -H 'Accept-Language: en-US,en;q=0.5' --compressed -H 'Connection: keep-alive' -H 'Referer: https://localhost:8080/login' 

done

