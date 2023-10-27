TS =
CXX = g++
BACKEND=libevent
BUILDCHAIN = make

BASE_CONTAINER = $(shell echo "wpp_$(CXX)_$(BACKEND)" | sed 's/++/pp/')
BASE_IMAGE = littlemole\/$(BASE_CONTAINER)

up: ## run the docker image and open a shell
	CXX=$(CXX) EVENTLIB=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) TS=$(TS) BASE_IMAGE=$(BASE_IMAGE) docker-compose up -d --build

force: 
	CXX=$(CXX) EVENTLIB=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) TS=$(TS) BASE_IMAGE=$(BASE_IMAGE) docker-compose build --no-cache
	CXX=$(CXX) EVENTLIB=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) TS=$(TS) BASE_IMAGE=$(BASE_IMAGE) docker-compose up -d --build --force-recreate
	

down: ## stop running docker image, if any
	CXX=$(CXX) EVENTLIB=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose stop

clean-logs:
	sh -c "truncate -s 0 /var/lib/docker/containers/*/*-json.log"
	
.PHONY: build up down update-dockerfile
