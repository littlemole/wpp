CXX = g++

TARGET=devenv
DESTDIR=/
PREFIX=/usr/local
WITH_TEST=Off
WITH_DEBUG=Off

PWD=$(shell pwd)

BACKEND = libevent
CONTAINER = $(shell echo "wpp_$(CXX)_$(BACKEND)" | sed 's/++/pp/')
IMAGE = littlemole/$(CONTAINER)

BASE_CONTAINER = $(shell echo "devenv_$(CXX)_cmake" | sed 's/++/pp/')
BASE_IMAGE = littlemole/$(BASE_CONTAINER)

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

PRESET_DBG = $(shell echo "gcc-debug-$(BACKEND)" | sed 's/boost_//' )
PRESET_REL = $(shell echo "gcc-release-$(BACKEND)" | sed 's/boost_//' )

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
	-rm -rf ./out
		
install: clean ## full build and install
	cmake --preset $(PRESET_DBG)
	cmake --build --preset $(PRESET_DBG)
	sudo cmake --build  --target install --preset $(PRESET_DBG)

	cmake --preset $(PRESET_REL)
	cmake --build --preset $(PRESET_REL)
	sudo cmake --build  --target install --preset $(PRESET_REL)

# docker stable testing environment

devenv:
	cd libraries/devenv && make -e -f Makefile image WITH_TEST=$(WITH_TEST) WITH_DEBUG=$(WITH_DEBUG) 


image: devenv ## build docker test image
	
	docker build -t $(IMAGE) . -fDockerfile  --build-arg CXX=$(CXX) --build-arg WITH_DEBUG=$(WITH_DEBUG) --build-arg EVENTLIB=$(BACKEND) --build-arg BASE_IMAGE=$(BASE_IMAGE)  --build-arg WITH_TEST=$(WITH_TEST)

clean-image: ## rebuild the docker test image from scratch
	cd libraries/devenv && make -e -f Makefile clean-image
	docker build -t $(IMAGE) . --no-cache -fDockerfile --build-arg CXX=$(CXX) --build-arg WITH_DEBUG=$(WITH_DEBUG) --build-arg EVENTLIB=$(BACKEND) --build-arg BASE_IMAGE=$(BASE_IMAGE) --build-arg WITH_TEST=$(WITH_TEST)
	
		
bash: rmc image ## run the docker image and open a shell
	docker run --name $(CONTAINER) -p9876:9876 -v ./libraries:/usr/local/src/wpp/libraries -ti $(IMAGE) bash 

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
	@echo "\t sudo make image CXX=clang++ DOCKERFILE=Dockerfile.fedora"
	@echo "available targets:"
	@grep -E -h '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

.PHONY: deb build help rmi rmc stop bash image clean-image release remove install test clean test-build