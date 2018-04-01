.PHONY: lint
lint:
	 clang-tidy src/require.cc -extra-arg-before=-xc++ -checks=-*,google-* -warnings-as-errors=* -- -std=c++11 -I/usr/lib/dart/include
	 ./cpplint.py --linelength=400 src/require.cc