
all:


build:
	@mkdir -p build \
	&& cd build \
	&& cmake .. \
	&& cmake --build . --config Release --target myapp -j $(nproc) --

build-test:
	@mkdir -p build \
	&& cd build \
	&& cmake .. \
	&& cmake --build . --config Release --target build-test -j $(nproc) --

run: build
	./build/myapp/myapp --server ./server_config.example.json

run-test: build-test
	@cd build \
	&& ctest

clean:
	@rm -rf build

build-image:
	docker build -f Dockerfile -t drogon-test-app .

run-container:
	docker run --rm -i -p 3000:3000 drogon-test-app:latest --server ./server_config.example.json

.PHONY: all build run build-image run-container