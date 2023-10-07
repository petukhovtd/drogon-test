
all:

build:
	docker build -f Dockerfile -t drogon-test-app .

run:
	docker run --rm -i -p 3000:3000 drogon-test-app:latest --server ./server_config.example.json

.PHONY: all build run