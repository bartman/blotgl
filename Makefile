.PHONY: all install coverage test help
.DEFAULT_GOAL := all

SHELL := /usr/bin/env bash

# build what we built last time, Release the first time, and can be overridden with TYPE= variable
DEFAULT_TYPE = Release
CURRENT_TYPE = $(shell sed -r -n -e 's/CMAKE_BUILD_TYPE:STRING=//p' build/CMakeCache.txt 2>/dev/null)

Q=$(if ${V},,@)

# user can override these
CMAKE ?= cmake
PREFIX ?= ~/.local
REBUILD ?= false
BUILD ?= build
TYPE ?= $(if ${CURRENT_TYPE},${CURRENT_TYPE},${DEFAULT_TYPE})
NPROC ?= $(shell nproc || echo 1)

all: ## [default] build the project (uses TYPE={Release,Debug})
	${Q}${CMAKE} -S. -B${BUILD} -DCMAKE_INSTALL_PREFIX="$(PREFIX)" -DCMAKE_BUILD_TYPE="${TYPE}"
	${Q}${CMAKE} --build "${BUILD}" --config "${TYPE}" --parallel "${NPROC}"
	${Q}ln -fs "${BUILD}"/compile_commands.json compile_commands.json

clean: ## clean out the build directory
	${Q}${CMAKE} --build "${BUILD}" --target clean

distclean: ## remove build directory completely
	${Q}rm -rf "${BUILD}"/

help:
	${Q}awk '/^[a-zA-Z_-]+:.*?## .*$$/ { split($$0, arr, ":.*?## "); printf "    %-20s %s\n", arr[1], arr[2] }' $(MAKEFILE_LIST)

test: ## run unit tests (with ctest, uses REBUILD={true,false})
	${Q}$(if $(filter 1 yes true YES TRUE,${REBUILD}),rm -rf "${BUILD}"/)
	${Q}${CMAKE} -S. -B${BUILD} -DCMAKE_INSTALL_PREFIX="$(PREFIX)" -DCMAKE_BUILD_TYPE="${TYPE}"
	${Q}${CMAKE} --build "${BUILD}" --config "${TYPE}" --parallel "${NPROC}"
	${Q}cd "${BUILD}"/ && ctest -C "${TYPE}" $(if ${CI},--output-on-failure -VV)
	${Q}for xml in ${BUILD}/*_report.xml ; do \
	  sed -r -i -e "s,$(shell pwd),,g" "$${xml}" ; \
	done
	${Q}echo " ✅ Unit tests complete."

BLOTGL       = ${BUILD}/src/blotgl
CAP_RUN_SH   = test/cap-run.sh
RUNNER_PY    = test/runner.py
SMOKE_JSON   = test/smoke/smoke.json
SMOKE_ENV    = 
SMOKE_VARS   = BLOTGL=${BLOTGL} INDIR=test/smoke OUTDIR=build/test/smoke
SMOKE_OUT    = ${BUILD}/test/smoke/smoke.log

smoke: ## run smoke test (uses REBUILD={true,false})
	${Q}$(if $(filter 1 yes true YES TRUE,${REBUILD}),rm -rf "${BUILD}"/)
	${Q}${CMAKE} -S. -B${BUILD} -DCMAKE_INSTALL_PREFIX="$(PREFIX)" -DCMAKE_BUILD_TYPE="${TYPE}"
	${Q}${CMAKE} --build "${BUILD}" --config "${TYPE}" --parallel "${NPROC}"
	${Q}mkdir -p $(dir ${SMOKE_OUT})
	${Q}if ! ${SMOKE_ENV} ${CAP_RUN_SH} $(if ${CI},,--quiet) ${SMOKE_OUT} ${RUNNER_PY} --blind-run --input ${SMOKE_JSON} ${SMOKE_VARS} ; \
		then grep --color=auto -e '^Warning:' -e '^Error:' ${SMOKE_OUT} ; \
	             echo >&2 " ❌ Smoke test failed, see ${SMOKE_OUT}" ; false ; \
		else echo >&2 " ✅ Smoke test passed!" ; fi

coverage: ## check code coverage (with gcov, uses REBUILD={true,false})
	${Q}$(if $(filter 1 yes true YES TRUE,${REBUILD}),rm -rf "${BUILD}"/)
	${Q}${CMAKE} -S. -B${BUILD} -DCMAKE_INSTALL_PREFIX="$(PREFIX)" -DCMAKE_BUILD_TYPE="${TYPE}"
	${Q}${CMAKE} --build "${BUILD}" --config "${TYPE}" --parallel "${NPROC}"
	${Q}cd "${BUILD}"/ && ctest -C "${TYPE}" -VV
	${Q}find "${BUILD}"/ -type f -name '*.gcno' -exec gcov -pb {} +

install: ## install the package (to the `PREFIX`, uses REBUILD={true,false})
	${Q}$(if $(filter 1 yes true YES TRUE,${REBUILD}),rm -rf "${BUILD}"/)
	${Q}${CMAKE} -S. -B${BUILD} -DCMAKE_INSTALL_PREFIX="$(PREFIX)" -DCMAKE_BUILD_TYPE="${TYPE}"
	${Q}${CMAKE} --build "${BUILD}" --config "${TYPE}" --parallel "${NPROC}"
	${Q}${CMAKE} --build "${BUILD}" --target install --config "${TYPE}"

format: ## format the project sources (uses REBUILD={true,false})
	${Q}$(if $(filter 1 yes true YES TRUE,${REBUILD}),rm -rf "${BUILD}"/)
	${Q}${CMAKE} -S. -B${BUILD} -DCMAKE_INSTALL_PREFIX="$(PREFIX)" -DCMAKE_BUILD_TYPE="${TYPE}"
	${Q}${CMAKE} --build "${BUILD}" --target clang-format

full-test: ## like test, but with a full rebuild
	${Q}${MAKE} REBUILD=1 test
full-coverage: ## like coverage, but with a full rebuild
	${Q}${MAKE} REBUILD=1 coverage
