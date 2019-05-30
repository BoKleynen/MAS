.PHONY: build test

build-image:
	docker build -t ns-3 .

build-debug:
	docker run \
		--rm \
		--interactive \
		--tty \
		--workdir /repos/ns-3/source/ns-3.29\
		--volume `pwd`/build:/repos/ns-3/source/ns-3.29/build \
		--volume `pwd`/ant-routing:/repos/ns-3/source/ns-3.29/src/ant-routing \
		ns-3 \
		/bin/bash -c "./waf configure --build-profile=debug --out=build/debug --enable-examples --enable-tests && ./waf"

build-optimized:
	docker run \
		--rm \
		--interactive \
		--tty \
		--workdir /repos/ns-3/source/ns-3.29\
		--volume `pwd`/build:/repos/ns-3/source/ns-3.29/build \
		--volume `pwd`/ant-routing:/repos/ns-3/source/ns-3.29/src/ant-routing \
		ns-3 \
		/bin/bash -c "./waf configure --build-profile=optimized --out=build/optimized --enable-examples && ./waf"

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

sh:
	docker run \
		--rm \
		--interactive \
		--tty \
		--workdir /repos/ns-3/source/ns-3.29\
		--volume `pwd`/build:/repos/ns-3/source/ns-3.29/build \
		--volume `pwd`/scratch:/repos/ns-3/source/ns-3.29/scratch \
		--volume `pwd`/ant-routing:/repos/ns-3/source/ns-3.29/src/ant-routing \
		--cap-add=SYS_PTRACE \
		ns-3 \
		/bin/bash

stat:
	docker run \
		--rm \
		--interactive \
		--tty \
		--workdir /repos/ns-3/source/ns-3.29\
		--volume `pwd`/build:/repos/ns-3/source/ns-3.29/build \
		--volume `pwd`/stats:/repos/ns-3/source/ns-3.29/examples/stats \
		--volume `pwd`/scratch:/repos/ns-3/source/ns-3.29/scratch \
		ns-3 \
		/bin/bash -c "./waf configure --build-profile=debug --out=build/debug --enable-examples --enable-tests && ./examples/stats/wifi-example-db.sh"
