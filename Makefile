.PHONY: run

run:
	docker run \
		--rm \
		--interactive \
		--tty \
		--workdir /repos/ns-3/source/ns-3.29\
		--volume `pwd`/ant-routing:/repos/ns-3/source/ns-3.29/src/ant-routing \
		--volume `pwd`/scratch:/repos/ns-3/source/ns-3.29/scratch \
		bokleynen/mas:optimized \
		/bin/bash -c "./waf configure --build-profile=optimized --out=build/optimized && ./waf --run mas"

build-image:
	docker build -t bokleynen/mas:optimized -f optimized-mas.Dockerfile .

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
