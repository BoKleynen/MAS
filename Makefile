.PHONY: build test

build:
	docker run \
		--rm \
		--interactive \
		--tty \
		--workdir /repos/ns-3/source/ns-3.29\
		--volume `pwd`/build:/repos/ns-3/source/ns-3.29/build \
		--volume `pwd`/ant-routing:/repos/ns-3/source/ns-3.29/src/ant-routing \
		ns-3 \
		/bin/bash -c "./waf configure --enable-examples --enable-tests && ./waf"

test:
	docker run \
		--rm \
		--interactive \
		--tty \
		--workdir /repos/ns-3/source/ns-3.29\
		--volume `pwd`/build:/repos/ns-3/source/ns-3.29/build \
		--volume `pwd`/ant-routing:/repos/ns-3/source/ns-3.29/ant-routing \
		ns-3 \
		/bin/bash -c './waf configure --enable-examples --enable-tests && ./waf --run "test-runner --suite=ant-packet"'
