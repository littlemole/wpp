CXX = g++
BACKEND=libevent
BUILDCHAIN = make

BASE_CONTAINER = $(shell echo "wpp_$(CXX)_$(BACKEND)" | sed 's/++/pp/')
BASE_IMAGE = littlemole\/$(BASE_CONTAINER)

build: ## run the docker image and open a shell
	CXX=$(CXX) BACKEND=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose build

up: ## run the docker image and open a shell
	CXX=$(CXX) BACKEND=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose up -d --build

down: ## stop running docker image, if any
	CXX=$(CXX) BACKEND=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose stop
	
.PHONY: build up down update-dockerfile
 
 