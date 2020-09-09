all: ../src/network_test_script.c ../src/net_test_script.c

../src/network_test_script.c: network_test.js
	./convert.rb network_test.js | tee ../src/network_test_script.c

../src/net_test_script.c: net_test.js
	./convert.rb net_test.js | tee ../src/net_test_script.c

