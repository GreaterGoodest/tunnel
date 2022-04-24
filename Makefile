all: basic_proxy

.PHONY: basic_proxy

basic_proxy:
	mkdir -p ./build
	cd build && cmake ../src/ && make 

clean:
	rm -rf build