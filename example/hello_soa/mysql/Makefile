PWD = $(shell pwd)

IMG=oauth-mysql
TAG="promise-mysql"

image:  
	sudo docker build -t $(TAG) .
        # --no-cache
	
run: image
	-sudo docker run --name $(IMG) -d -e "MYSQL_ALLOW_EMPTY_PASSWORD=yes"  $(TAG) 
                                        
bash:                                   
	sudo docker exec -ti $(IMG) bash

stop:   
	-sudo docker stop $(IMG)
	

remove: stop
	-sudo docker rm $(IMG)	