CXX = g++
BACKEND=libevent
BUILDCHAIN = cmake

CONTAINER = $(shell echo "reproweb_example_$(CXX)_$(BACKEND)_$(BUILDCHAIN)" | sed 's/++/pp/')
BASE_CONTAINER = $(shell echo "wpp_$(CXX)_$(BACKEND)" | sed 's/++/pp/')
IMAGE = littlemole/$(CONTAINER)
BASE_IMAGE = littlemole\/$(BASE_CONTAINER)


cacert :
	cd pem && ./make_cert.sh
	
image: 
	docker build -t $(IMAGE) . -fDockerfile  --build-arg CXX=$(CXX) --build-arg BUILDCHAIN=$(BUILDCHAIN) --build-arg BACKEND=$(BACKEND) --build-arg BASE_IMAGE=$(BASE_IMAGE)

bash: image
	docker run --name hello_world -ti -e CXX=$(CXX) -e BUILDCHAIN=$(BUILDCHAIN) -e BACKEND=$(BACKEND)  $(IMAGE) bash

build: ## build the docker image
	CXX=$(CXX) BACKEND=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose build

up: ## run the docker image 
	echo $(BUILDCHAIN)
	CXX=$(CXX) BACKEND=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose up -d --build

force: ## run the docker image (no cache)
	CXX=$(CXX) BACKEND=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose build --no-cache
	CXX=$(CXX) BACKEND=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose up -d --build

down: ## stop running docker image, if any
	CXX=$(CXX) BACKEND=$(BACKEND) BUILDCHAIN=$(BUILDCHAIN) BASE_IMAGE=$(BASE_IMAGE) docker-compose stop

# self documenting makefile, see 
# https://marmelab.com/blog/2016/02/29/auto-documented-makefile.html

help:
	@echo "common build flags for all targets:"
	@echo "\tCXX set to g++|clang++ to set compiler (defaults to g++)"
	@echo "\tBACKEND set to libevent|boost_asio to set event backend"
	@echo "example calls:"
	@echo "\t make up CXX=clang++ "
	@echo "\t make up CXX=clang++ BACKEND=boost_asio "
	@echo "available targets:"
	@grep -E -h '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
		
.PHONY: build up force down cacert help
 
 