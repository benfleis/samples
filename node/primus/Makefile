NMPATH ?= ./node_modules/.bin

COFFEE ?= ${NMPATH}/coffee
COFFEE_DEBUG ?= ${COFFEE} --nodejs debug

MOCHA ?= ${NMPATH}/mocha
MOCHA_FLAGS += --compilers coffee:coffee-script/register --no-colors --reporter spec

TEST_FLAGS ?= test/*_tests.coffee

# -----------------------------------------------------------------------------

setup:
	npm install .

compile:
	${COFFEE} -wmc -o lib .

debug:
	${COFFEE_DEBUG} index.coffee

run:
	${COFFEE} index.coffee

clean:
	rm -f index.js index.map

tags:
	ctags -R src

.PHONY:	setup clean compile debug run tags
