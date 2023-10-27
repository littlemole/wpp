CXX = g++

TARGET=devenv
DESTDIR=/
PREFIX=/usr/local

PWD=$(shell pwd)

BUILDCHAIN = make
CONTAINER = $(shell echo "devenv_$(CXX)_$(BUILDCHAIN)" | sed 's/++/pp/')
IMAGE = littlemole/$(CONTAINER)
WITH_TEST = On

DOCKERFILE=Dockerfile

#################################################
# rule to compile all (default rule)
#################################################

all: ## just compiles and links the library
	make help
	
#################################################
# make clean
#################################################

clean: ## cleans up build artefacts
	-find -name "*~" -exec rm {} \;
	-rm -rf ./build
		
# docker stable testing environment

image: ## build docker test image
	docker build -t $(IMAGE) . -f $(DOCKERFILE)  --build-arg CXX=$(CXX) --build-arg BUILDCHAIN=$(BUILDCHAIN) --build-arg WITH_TEST=$(WITH_TEST)

clean-image: ## rebuild the docker test image from scratch
	docker build -t $(IMAGE) . --no-cache -f $(DOCKERFILE) --build-arg CXX=$(CXX) --build-arg BUILDCHAIN=$(BUILDCHAIN) --build-arg WITH_TEST=$(WITH_TEST)
		
bash: rmc image ## run the docker image and open a shell
	docker run --name $(CONTAINER) -ti $(IMAGE) bash

stop: ## stop running docker image, if any
	-docker stop $(CONTAINER)
	
rmc: stop ## remove docker container, if any
	-docker rm $(CONTAINER)

rmi : ## remove existing docker image, if any
	-docker rmi $(IMAGE)

# self documenting makefile, see 
# https://marmelab.com/blog/2016/02/29/auto-documented-makefile.html

help:
	@echo "common build flags for all targets:"
	@echo "\tCXX set to g++|clang++ to set compiler (defaults to g++)"
	@echo "example calls:"
	@echo "\t sudo make image CXX=clang++ "
	@echo "\t sudo make image CXX=clang++ BUILDCHAIN=cmake "
	@echo "available targets:"
	@grep -E -h '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

.PHONY: deb build help rmi rmc stop bash image clean-image release remove install test clean test-build