.PHONY: build test

build-image:
	docker build -t ns-3 .

build:
	docker run \
		--rm \
		--interactive \
		--tty \
		--workdir /repos/ns-3/source/ns-3.29\
		--volume `pwd`/build:/repos/ns-3/source/ns-3.29/build \
		--volume `pwd`/ant-routing:/repos/ns-3/source/ns-3.29/src/ant-routing \
		ns-3 \
		/bin/bash -c "./waf configure --enable-examples --enable-tests && ./waf -d debug"

test:
	docker run \
		--rm \
		--interactive \
		--tty \
		--workdir /repos/ns-3/source/ns-3.29\
		--volume `pwd`/build:/repos/ns-3/source/ns-3.29/build \
		--volume `pwd`/ant-routing:/repos/ns-3/source/ns-3.29/src/ant-routing \
		--volume `pwd`/scratch:/repos/ns-3/source/ns-3.29/scratch \
		ns-3 \
		/bin/bash -c './waf configure --enable-examples --enable-tests && ./waf --run "test-runner --suite=ant-packet"'

run-aodv: build
	docker run \
		--rm \
		--interactive \
		--tty \
		--workdir /repos/ns-3/source/ns-3.29\
		--volume `pwd`/build:/repos/ns-3/source/ns-3.29/build \
		--volume `pwd`/ant-routing:/repos/ns-3/source/ns-3.29/src/ant-routing \
		--volume `pwd`/scratch:/repos/ns-3/source/ns-3.29/scratch \
		ns-3 \
		/bin/bash -c "./waf --run scratch/aodv"
