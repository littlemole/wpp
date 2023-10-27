CXX = g++
BACKEND=libevent
BUILDCHAIN = make

CONTAINER = $(shell echo "reproweb_example_$(CXX)_$(BACKEND)_$(BUILDCHAIN)" | sed 's/++/pp/')
BASE_CONTAINER = $(shell echo "wpp_$(CXX)_$(BACKEND)" | sed 's/++/pp/')
IMAGE = littlemole/$(CONTAINER)
BASE_IMAGE = littlemole\/$(BASE_CONTAINER)

build: ## run the docker image and open a shell
	CXX=$(CXX) EVENTLIB=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose build

up: ## run the docker image and open a shell
	CXX=$(CXX) EVENTLIB=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose up -d --build

down: ## stop running docker image, if any
	CXX=$(CXX) EVENTLIB=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose stop
	
.PHONY: build up down 
 
 