CXX = g++
BACKEND=libevent
BUILDCHAIN = cmake

BASE_CONTAINER = $(shell echo "wpp_$(CXX)_$(BACKEND)" | sed 's/++/pp/')
BASE_IMAGE = littlemole\/$(BASE_CONTAINER)

up: ## run the docker image and open a shell
	CXX=$(CXX) EVENTLIB=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose up -d --build

force: ## run the docker image and open a shell
	CXX=$(CXX) EVENTLIB=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose build --no-cache
	CXX=$(CXX) EVENTLIB=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose up -d --build

clean:
	docker-compose down
	docker-compose rm -f

down: ## stop running docker image, if any
	CXX=$(CXX) EVENTLIB=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) docker-compose stop

clean-logs:
	sh -c "truncate -s 0 /var/lib/docker/containers/*/*-json.log"
	
cacert :
	cd ca && bin/make_pki

.PHONY: build up down update-dockerfile cacert
