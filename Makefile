MAKEFLAGS += --silent
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --jobs $(shell grep -c ^processor /proc/cpuinfo)

name := $(shell basename $$PWD)
msrc := main.cpp
src := $(shell find -type f -name "*.cpp" -and -not -name $(msrc) -printf "%P\n")
src := $(sort $(src))
src += $(msrc)
dsrc := $(shell find -type f -name "*.cxx" -printf "%P\n")
dsrc := $(sort $(dsrc))

tmp := /run/user/$(shell id -u)/make-$(name)
$(info tmp: $(tmp))

obj = $(patsubst %,$(tmp)/%.o,$(src))
dobj = $(patsubst %,$(tmp)/%.o,$(dsrc))

c = gcc
std = -std=c++23
# inc = -I../_inc -I../_dbimage
inc =
war = -Wall -Wno-overloaded-virtual -ftabstop=4 -Wno-misleading-indentation -Wextra
lib = -lrt -lc -lstdc++
fopt = -fPIC -fmax-errors=10 -fdiagnostics-color=always
opt0 = -g -Og -rdynamic
opt3 = -O3 -rdynamic
armc = arm-linux-gnueabihf-$(c)
armarch = -marm -mtune=arm1176jzf-s
arm64c = aarch64-linux-gnu-$(c)
arm64arch =
armlib = -static

.PHONY: compile run release arm arm64 doc clean all deploy

compile: $(obj) $(dobj)
	echo $(@)

all: release arm arm64 doc

deploy: deploy_pi0 deploy_odroidm1

deploy_odroidm1: arm64
	echo "deploying to odroid@odroidm1:$$(pwd)/" ; \
	rsync --verbose --times \
	      draw img draw-out img-out draw-bal img-bal wtc dtu grep-stat \
	      odroid@odroidm1:$$(pwd)/

deploy_pi0: arm
	echo "deploying to pi@pi0:$$(pwd)/" ; \
	rsync --verbose --times \
	      $(name)-arm draw img draw-out img-out draw-bal img-bal wtc dtu grep-stat err.sh log \
	      pi@pi0:$$(pwd)/

run: $(tmp)/$(name)
	echo $(@): $(tmp)/$(name)

release: $(name)
	echo $(@)

arm: $(name)-arm
	echo $(@)

arm64: $(name)-arm64
	echo $(@)

doc: $(tmp)/doc/timestamp 
	echo $(tmp)/doc/html/index.html

$(tmp)/doc/doxyfile: doxyfile $(tmp)/doc
	sed -e 's#PROJECT_NAME\s*=\s*.*#PROJECT_NAME = $(name)#' \
	    -e 's#OUTPUT_DIRECTORY\s*=\s*.*#OUTPUT_DIRECTORY = $(tmp)/doc/#' \
	    -e "s|PROJECT_NUMBER\\s*=.*|PROJECT_NUMBER = $$(date "+%Y-%m-%d %H:%M:%S")|" \
	    $(<) > $(@)

$(tmp)/doc/timestamp: $(tmp)/doc/doxyfile $(src) $(dsrc) $(tmp)
	doxygen $(<) && touch $(@)

clean:
	echo $(@) ; \
	rm -rf "$(tmp)" "$(name)" "$(name)-arm" "$(name)-arm64" ; \
	echo " lines  words  bytes file" ; \
	find -type f -not -name "*.txt" -not -name "doxyfile" -printf "%P\n" | sort | xargs wc
	echo -n "files: " ; \
	find -type f -not -name "*.txt" -not -name "doxyfile" -printf "%P\n" | sort | wc -l

$(tmp):
	mkdir $(tmp)

$(tmp)/doc: $(tmp)
	mkdir $(tmp)/doc

$(tmp)/%.cpp.o: %.cpp $(tmp)
	echo $(<) ; \
	rm -f $(@) ; \
	out="$$($(c) $(std) $(inc) $(war) $(fopt) $(<) $(opt0) -c -o $(@) 2>&1)" ; \
	ret=$$? ; \
	echo -n "$$out" | sed -e s"#^#$$(printf "%16s" $(patsubst %.cpp,%,$(<))):  #" ; \
	exit $$ret

$(tmp)/%.cxx.o: %.cxx $(tmp)
	echo $(<) ; \
	mkdir -p $$(dirname $(@)) ; \
	out="$$(touch -r $(<) $(@) 2>&1)" ; \
	ret=$$? ; \
	echo -n "$$out" ; \
	exit $$ret

$(tmp)/$(name): $(src) $(dsrc) $(tmp)
	echo $(@) ; \
	$(c) $(std) $(inc) $(war) $(fopt) $(msrc) $(opt0) $(lib) -o $(@)

$(name): $(src) $(dsrc)
	echo $(@) ; \
	rm -f $(@) ; \
	if [ ! -z "$(rp)" ] ; then export PATH="$(rp):$${PATH}" ; fi ; \
	$(c) --version | head -n 1 ; \
	$(c) $(std) $(inc) $(war) $(fopt) $(msrc) $(opt3) $(lib) -o $(@)

$(name)-arm: $(src) $(dsrc)
	echo $(@) ; \
	rm -f $(@) ; \
	echo $(armc) $(std) $(armarch) $(inc) $(war) $(fopt) $(msrc) $(opt3) $(lib) $(armlib) -o $(@) ; \
	$(armc) --version | head -n 1 ; \
	$(armc) $(std) $(armarch) $(inc) $(war) $(fopt) $(msrc) $(opt3) $(lib) $(armlib) -o $(@)

$(name)-arm64: $(src) $(dsrc)
	echo $(@) ; \
	rm -f $(@) ; \
	echo $(arm64c) $(std) $(arm64arch) $(inc) $(war) $(fopt) $(msrc) $(opt3) $(lib) $(armlib) -o $(@) ; \
	$(arm64c) --version | head -n 1 ; \
	$(arm64c) $(std) $(arm64arch) $(inc) $(war) $(fopt) $(msrc) $(opt3) $(lib) $(armlib) -o $(@)

