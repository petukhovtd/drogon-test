SOURCE_DIR=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR=$(SOURCE_DIR)/build/
APP_TARGET=myapp
BUILD_TEST_TARGET=build-test

$(info SOURCE_DIR=$(SOURCE_DIR))
$(info BUILD_DIR=$(BUILD_DIR))

define in_build
	@cd "$(BUILD_DIR)" ; \
	$(1) ; \
	cd "$(SOURCE_DIR)"
endef

all:

$(BUILD_DIR):
	@mkdir -p "$(BUILD_DIR)"
	$(call in_build,cmake ..)

build: $(BUILD_DIR)
	$(call in_build,cmake --build . --config Release --target $(APP_TARGET) -j $(nproc))

build-test: $(BUILD_DIR)
	$(call in_build,cmake --build . --config Release --target $(BUILD_TEST_TARGET) -j $(nproc))

run: build
	./build/myapp/myapp --server ./server_config.example.json

run-test: build-test
	$(call in_build,ctest)

deconfig:
	@rm -rfv "$(BUID_DIR)"

build-image:
	docker build -f Dockerfile -t drogon-test-app .

run-container:
	docker run --rm -i -p 3000:3000 drogon-test-app:latest --server ./server_config.example.json

.PHONY: all build build-test run run-test deconfig build-image run-container